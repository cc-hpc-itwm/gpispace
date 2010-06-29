/*
 * =====================================================================================
 *
 *       Filename:  layer.hpp
 *
 *    Description:  workflow engine management layer
 *
 *        Version:  1.0
 *        Created:  02/25/2010 12:51:21 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_LAYER_HPP
#define WE_MGMT_LAYER_HPP 1

#include <cassert>

#include <fhglog/fhglog.hpp>

#include <we/net.hpp>
#include <we/util/warnings.hpp>

#include <boost/random.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/serialization/access.hpp>

#include <we/mgmt/basic_layer.hpp>

#include <we/mgmt/exception.hpp>
#include <we/mgmt/bits/traits.hpp>
#include <we/mgmt/bits/policy.hpp>
#include <we/mgmt/bits/commands.hpp>
#include <we/mgmt/bits/synch_net.hpp>
#include <we/mgmt/bits/queue.hpp>
#include <we/mgmt/bits/set.hpp>
#include <we/mgmt/bits/signal.hpp>
#include <we/mgmt/bits/descriptor.hpp>

namespace we { namespace mgmt {
    namespace exception
    {
      struct validation_error : public std::runtime_error
      {
        validation_error (const std::string & msg, const std::string & act_name)
          : std::runtime_error (msg)
          , name (act_name)
        {}

        ~validation_error () throw ()
        {}

        const std::string name;
      };

      template <typename ExternalId>
      struct already_there : public std::runtime_error
      {
        already_there (const std::string & msg, ExternalId const & ext_id)
          : std::runtime_error (msg)
          , id (ext_id)
        { }

        ~already_there () throw ()
        { }

        const ExternalId id;
      };

      template <typename IdType>
      struct no_such_mapping : public std::runtime_error
      {
        no_such_mapping (const std::string & msg, IdType const & an_id)
          : std::runtime_error (msg)
          , id (an_id)
        {}

        ~no_such_mapping () throw ()
        {}

        const IdType id;
      };
    }

    template < typename IdType
             , typename Activity
             , typename Traits = traits::layer_traits<Activity>
             , typename Policy = policy::layer_policy<Traits>
             >
    class layer : public basic_layer<IdType>
    {
    public:
      typedef layer<IdType, Activity, Traits, Policy> this_type;
      // external ids
      typedef IdType id_type;
      typedef id_type external_id_type;

      typedef Activity activity_type;
      typedef Traits traits_type;
      typedef Policy policy;

      // internal ids
      typedef typename traits_type::id_traits  internal_id_traits;
      typedef typename internal_id_traits::type internal_id_type;

      typedef std::string encoded_type;
      typedef std::string reason_type;
      typedef std::string result_type;
      typedef std::string status_type;

      typedef typename activity_type::output_t output_type;

    private:
      typedef boost::unordered_set<internal_id_type> child_set_t;
      typedef boost::unordered_map<internal_id_type, internal_id_type> child_to_parent_map_t;
      typedef boost::unordered_map<internal_id_type, child_set_t> parent_to_children_map_t;

      typedef boost::unordered_map<external_id_type, internal_id_type> external_to_internal_map_t;
      typedef boost::unordered_map<internal_id_type, external_id_type> internal_to_external_map_t;

      typedef std::vector<boost::thread *> thread_list_t;

      typedef detail::descriptor<activity_type, internal_id_type, external_id_type> descriptor_type;
      typedef typename boost::unordered_map<internal_id_type, descriptor_type> activities_t;

      // manager thread
      typedef detail::commands::command_t<detail::commands::CMD_ID, internal_id_type> cmd_t;
      typedef detail::queue<cmd_t, policy::COMMAND_QUEUE_SIZE> cmd_q_t;

      // injector thread
      typedef internal_id_type inj_cmd_t;
      typedef detail::queue<inj_cmd_t, policy::INJECTOR_QUEUE_SIZE> inj_q_t;

      // extractor
      typedef detail::set<internal_id_type, policy::EXTRACTOR_QUEUE_SIZE> active_nets_t;

      typedef boost::unique_lock<boost::recursive_mutex> lock_t;
    public:

      /******************************
       * EXTERNAL API
       *****************************/

      // observe
      util::signal<void (const this_type *, internal_id_type const &)> sig_submitted;
      util::signal<void (const this_type *, internal_id_type const &, std::string const &)> sig_finished;
      util::signal<void (const this_type *, internal_id_type const &, std::string const &)> sig_failed;
      util::signal<void (const this_type *, internal_id_type const &, std::string const &)> sig_cancelled;
      util::signal<void (const this_type *, internal_id_type const &)> sig_executing;
      util::signal<void (const this_type *, internal_id_type const &)> sig_suspended;
      util::signal<void (const this_type *, internal_id_type const &)> sig_resumed;

      /**
       * Submit a new petri net to the petri-net management layer
       *
       *	  pre-conditions: none
       *
       *	  side-effects: parses the passed data
       *					registeres the petri-net with the mgmt layer
       *
       *	  post-conditions: the net is registered is with id "id"
       *
       */
      void submit(const external_id_type & id, const encoded_type & bytes) throw (std::exception)
      {
        DLOG(TRACE, "submit (" << id << ", ...)");

        activity_type act = policy::codec::decode(bytes);
        descriptor_type desc (generate_internal_id(), act);
        desc.came_from_external_as (id);
        desc.inject_input ();

        submit (desc);
      }

      /**
       * Inform the management layer to cancel a previously submitted net
       *
       *	  pre-conditions:
       *		  - a network must have been registered and assigned the id "id"
       *
       *	  side-effects:
       *		  - the hierarchy belonging to the net is cancelled in turn
       *
       *	  post-conditions:
       *		  - the internal state of the network switches to CANCELLING
       *		  - all children of the network will be terminated
       * */
      bool cancel(const external_id_type & id, const reason_type & reason) throw ()
      {
        DLOG(TRACE, "cancel (" << id << ", " << reason << ")");

        we::util::remove_unused_variable_warning(reason);

        post_cancel_activity_notification (map_to_internal(id));
        return true;
      }

      /**
       * Inform the management layer that an execution finished with the given result
       *
       *	  pre-conditions:
       *		  - the management layer submitted an activity to be executed with id "id"
       *
       *	  side-effects:
       *		  -	the results are integrated into the referenced net
       *
       *	  post-conditions:
       *		  - the node belonging to this is activity is removed
       **/
      bool finished(const external_id_type & id, const result_type & result) throw()
      {
        internal_id_type int_id (map_to_internal (id));
        {
          lock_t lock(mutex_);
          descriptor_type & desc (lookup (int_id));
          {
            activity_type act (policy::codec::decode (result));
            DLOG(TRACE, "finished (" << desc.name() << ")-" << id << ": " << ::util::show(act.output().begin(), act.output().end()));
            desc.output (act.output());
          }
        }

        post_finished_notification (int_id);
        return true;
      }

      /**
       * Inform the management layer that an execution failed with the given result
       *
       *	  pre-conditions:
       *		  - the management layer submitted an activity to be executed with id "id"
       *
       *	  side-effects:
       *		  -	the results are integrated into the referenced net
       *
       *	  post-conditions:
       *		  - the node belonging to this activity is removed
       **/
      bool failed ( const external_id_type & id
                  , const result_type & result
                  ) throw()
      {
        DLOG(TRACE, "failed (" << id << ", " << result << ")");

        we::util::remove_unused_variable_warning(result);

        post_failed_notification (map_to_internal(id));
        return true;
      }

      /**
       * Inform the management layer that an execution has been cancelled
       *
       *	  pre-conditions:
       *		  - the management layer submitted an activity to be executed with id "id"
       *		  - the management layer requested the cancellation of an activity
       *
       *	  side-effects:
       *		  - the enclosing workflow will be informed that an activity has been cancelled
       *
       *	  post-conditions:
       *		  - the node belonging to this activity is removed
       **/
      bool cancelled(const external_id_type & id) throw()
      {
        DLOG(TRACE, "cancelled (" << id << ")");

        post_cancelled_notification (map_to_internal(id));
        return true;
      }

      /**
       * Temporarily suspend the execution of the given petri-net
       *
       *	  pre-conditions:
       *		  - a net with the given id had been submitted
       *
       *	  side-effects:
       *		  - sub-activities are suspended as well
       *
       *	  post-conditions:
       *		  - the network will not be considered in the selection of new activities
       *		  - the execution of the network is on hold
       */
      bool suspend(const external_id_type & id) throw()
      {
        DLOG(TRACE, "suspend (" << id << ")");

        post_suspend_activity_notification (map_to_internal(id));
        return true;
      }

      /**
       * Execution of a network is resumed.
       *
       *	  pre-conditions:
       *		  - a net with the given id had been submitted
       *
       *	  side-effects:
       *		  - sub-activities are resumed as well
       *
       *	  post-conditions:
       *		  - the network will again be considered in the selection of new activities
       *
       */
      bool resume(const external_id_type & id) throw()
      {
        DLOG(TRACE, "resume (" << id << ")");

        post_suspend_activity_notification (map_to_internal(id));
        return true;
      }

      // END: EXTERNAL API

      status_type status(const external_id_type & id) throw (std::exception)
      {
        return ::util::show (lookup (map_to_internal(id)));
      }

      void print_statistics (std::ostream & s) const
      {
        s << "==== begin layer statistics ====" << std::endl;

        lock_t lock (mutex_);
        s << "   #activities := " << activities_.size() << std::endl;
        s << "   ext -> int := " << ::util::show (ext_to_int_.begin(), ext_to_int_.end());
        s << std::endl;
        s << std::endl;

        std::vector <internal_id_type> ids;
        ids.reserve (activities_.size());

        for ( typename activities_t::const_iterator desc (activities_.begin())
            ; desc != activities_.end()
            ; ++desc
            )
        {
          ids.push_back (desc->first);
        }

        std::sort (ids.begin(), ids.end());

        for ( typename std::vector<internal_id_type>::const_iterator id (ids.begin())
            ; id != ids.end()
            ; ++id
            )
        {
          s << activities_.at(*id);
          s << std::endl;
        }

        s << std::endl;
        s << "==== end layer statistics ====" << std::endl;
      }
    private:
      // handle execution layer
      boost::function<void (external_id_type const &, encoded_type const &)> ext_submit;
      boost::function<bool (external_id_type const &, reason_type const &)>  ext_cancel;
      boost::function<bool (external_id_type const &, result_type const &)>  ext_finished;
      boost::function<bool (external_id_type const &, result_type const &)>  ext_failed;
      boost::function<bool (external_id_type const &)>                       ext_cancelled;

      void submit (const descriptor_type & desc)
      {
        try
        {
          policy::validator::validate (desc.activity());
          insert_activity(desc);

          sig_submitted (this, desc.id());
          post_execute_notification (desc.id());
        }
        catch (const std::exception & ex)
        {
          throw exception::validation_error( std::string("layer::submit(): invalid activity:")
                                           + " id := " + ::util::show(desc.id())
                                           + " name := " + desc.name()
                                           , desc.name()
                                           );
        }
      }

      void add_map_to_internal ( const external_id_type & external_id
                               , const internal_id_type & internal_id
                               )
      {
        lock_t lock (mutex_);
        typename external_to_internal_map_t::const_iterator mapping
          (ext_to_int_.find(external_id));

        if (mapping != ext_to_int_.end())
        {
          throw exception::already_there<external_id_type>
            ( "already_there: ext_id := "
            + ::util::show(external_id)
            + " -> int_id := "
            + ::util::show(mapping->second)
            , external_id
            );
        }

        ext_to_int_.insert ( typename external_to_internal_map_t::value_type
                           (external_id, internal_id)
                           );
        DLOG(TRACE, "added mapping " << external_id << " -> " << internal_id);
      }

      void del_map_to_internal ( const external_id_type & external_id
                               , const internal_id_type & internal_id
                               )
      {
        lock_t lock (mutex_);

        typename external_to_internal_map_t::const_iterator mapping
          (ext_to_int_.find(external_id));
        if (  mapping == ext_to_int_.end()
           || mapping->second != internal_id
           )
        {
          throw exception::no_such_mapping<external_id_type>
            ( "no_such_mapping: ext := "
            + ::util::show(external_id)
            + " -> int := "
            + ::util::show(internal_id)
            , external_id
            );
        }

        ext_to_int_.erase (external_id);

        DLOG(TRACE, "deleted mapping " << external_id << " -> " << internal_id);
      }

      typename external_to_internal_map_t::mapped_type map_to_internal ( const external_id_type & external_id ) const
      {
        lock_t lock (mutex_);

        typename external_to_internal_map_t::const_iterator mapping (ext_to_int_.find(external_id));
        if (mapping != ext_to_int_.end())
        {
          return mapping->second;
        }
        else
        {
          throw exception::no_such_mapping<external_id_type> ("no_such_mapping: ext_id := " + ::util::show (external_id), external_id);
        }
      }
    public:
      /**
       * Constructor calls
       */
      layer()
        : sig_submitted("sig_submitted")
        , sig_finished("sig_finished")
        , sig_failed("sig_finished")
        , sig_cancelled("sig_finished")
        , sig_executing("sig_finished")
        , sig_suspended("sig_suspended")
        , sig_resumed("sig_resumed")
        , internal_id_gen_(&internal_id_traits::generate)
      {
        start();
      }

      template <class E, typename G>
      layer(E * exec_layer, G gen)
        : sig_submitted("sig_submitted")
        , sig_finished("sig_finished")
        , sig_failed("sig_finished")
        , sig_cancelled("sig_finished")
        , sig_executing("sig_finished")
        , sig_suspended("sig_suspended")
        , sig_resumed("sig_resumed")
        , external_id_gen_(gen)
        , internal_id_gen_(&internal_id_traits::generate)
      {
        connect (exec_layer);
        start();
      }

      template <typename T>
      void connect ( T * t )
      {
        ext_submit = (boost::bind (& T::submit, t, _1, _2));
        ext_cancel = (boost::bind (& T::cancel, t, _1, _2));
        ext_finished = (boost::bind (& T::finished, t, _1, _2));
        ext_failed = (boost::bind (& T::failed, t, _1, _2));
        ext_cancelled = (boost::bind (& T::cancelled, t, _1));
      }

      template <typename IdGen>
      void set_id_generator ( IdGen gen )
      {
        lock_t gen_lock (id_gen_mutex_);

        external_id_gen_ = gen;
      }

      ~layer()
      {
        // stop threads
        stop();

        // cancel external activities

        // clean up all activities
        while (! activities_.empty())
        {
          std::cerr << "D: removing act[" << activities_.begin()->first  << "]" << std::endl;
          activities_.erase (activities_.begin());
        }
        ext_to_int_.clear();
      }

      /* internal functions */
    private:
      void start()
      {
        LOG(DEBUG, "Workflow Management layer starting...");

        lock_t lock (mutex_);

        manager_   = boost::thread(boost::bind(&this_type::manager, this));

        const std::size_t num_extractors(2);
        active_nets_ = new active_nets_t[num_extractors];
        start_threads (num_extractors, extractor_, boost::bind(&this_type::extractor, this, _1));

        const std::size_t num_injectors(2);
        inj_q_ = new active_nets_t[num_injectors];
        start_threads (num_injectors, injector_, boost::bind(&this_type::injector, this, _1));
      }

      template <typename ThreadList, typename ThreadFunc>
      void start_threads( const std::size_t num, ThreadList & list, ThreadFunc tf)
      {
        for (std::size_t rank(0); rank < num; ++rank)
        {
          list.push_back (new boost::thread (boost::bind (tf, rank)));
        }
      }

      template <typename ThreadList>
      void stop_threads( ThreadList & list )
      {
        for (typename ThreadList::iterator t (list.begin()); t != list.end(); ++t)
        {
          (*t)->interrupt();
          (*t)->join();
          delete (*t);
        }
        list.clear();
      }

      void stop()
      {
        LOG(DEBUG, "Workflow Management layer stopping...");

        DLOG(TRACE, "cleaning up manager thread...");
        manager_.interrupt();
        manager_.join();

        DLOG(TRACE, "cleaning up injector threads...");
        stop_threads (injector_);

        DLOG(TRACE, "cleaning up extractor threads...");
        stop_threads (extractor_);

        delete [] active_nets_;
        active_nets_ = NULL;

        delete [] inj_q_;
        inj_q_ = NULL;
      }

      void manager()
      {
        using namespace we::mgmt::detail::commands;
        std::cerr << "D: manager thread started..." << std::endl;
        for (;;)
        {
          cmd_t cmd = cmd_q_.get();
          try
          {
            cmd.handle();
          }
          catch (std::exception const& ex)
          {
            LOG(FATAL, "error during manager command handling: " << ex.what());
            throw;
          }
        }
        DLOG(TRACE, "D: manager thread stopped...");
      }

      inline
      void post_activity_notification (const internal_id_type & id)
      {
        active_nets_[id % extractor_.size()].put(id);
      }

      inline
      void post_inject_activity_results (const internal_id_type & id)
      {
        inj_q_[id % injector_.size()].put ( id );
      }

      inline
      void post_finished_notification (const internal_id_type & id)
      {
        inj_q_[id % injector_.size()].put ( id );
      }

      inline
      void post_failed_notification (const internal_id_type & id)
      {
        cmd_q_.put(make_cmd(id, boost::bind(&this_type::activity_failed, this, _1)));
      }

      inline
      void post_cancelled_notification (const internal_id_type & id)
      {
        cmd_q_.put(make_cmd(id, boost::bind(&this_type::activity_cancelled, this, _1)));
      }

      inline
      void post_cancel_activity_notification (const internal_id_type & id)
      {
        cmd_q_.put (make_cmd(id, boost::bind(&this_type::cancel_activity, this, _1)));
      }

      inline
      void post_suspend_activity_notification (const internal_id_type & id)
      {
        cmd_q_.put (make_cmd(id, boost::bind(&this_type::suspend_activity, this, _1)));
      }

      inline
      void post_resume_activity_notification (const internal_id_type & id)
      {
        cmd_q_.put (make_cmd(id, boost::bind(&this_type::resume_activity, this, _1)));
      }

      inline
      void post_remove_activity_notification (const internal_id_type & id)
      {
        cmd_q_.put (make_cmd(id, boost::bind(&this_type::remove_activity_by_id, this, _1)));
      }

      inline
      void post_execute_notification (const internal_id_type & id)
      {
        active_nets_[id % extractor_.size()].put(id);
      }

      inline
      void execute_externally (const internal_id_type & int_id)
      {
        descriptor_type & desc (lookup (int_id));

        // create external id
        external_id_type ext_id ( generate_external_id() );
        desc.sent_to_external_as (ext_id);
        add_map_to_internal (ext_id, int_id);

        // copy the activity and let it be handled internally on the next level
        activity_t ext_act (desc.activity());
        ext_act.transition().set_internal(true);

        DLOG(DEBUG, "submitting internal activity " << int_id << " to external with id " << ext_id);
        ext_submit ( ext_id, policy::codec::encode (ext_act));
      }

      void extractor(const std::size_t rank)
      {
        DLOG(INFO, "extractor[" << rank << "] thread started...");
        for (;;)
        {
          internal_id_type active_id = active_nets_[rank].get();

          try
          {
            descriptor_type & desc = lookup(active_id);
            DLOG(TRACE, "extractor-" << rank << " puts attention to activity " << active_id);

            // TODO: check status flags
            if (! desc.is_alive ())
            {
              LOG(DEBUG, "activity (" << desc.name() << ")-" << active_id << " is on hold");
              continue;
            }

            // classify/execute
            //    EXTRACT: while loop
            //       classify/execute
            //          EXTRACT: remember id
            //       INJECT: inject
            //       EXTERN: extern
            //    INJECT:  inject to parent / notify client
            //    EXTERN:  send to extern
            typename policy::exec_policy exec_policy;

            sig_executing (this, desc.id());

            switch (desc.execute (exec_policy))
            {
            case policy::exec_policy::EXTRACT:
              {
                DLOG(TRACE, "extracting from net");
                while (desc.enabled())
                {
                  descriptor_type child (desc.extract (generate_internal_id()));
                  child.inject_input ();

                  DLOG(INFO, "extractor[" << rank << "]: extracted from (" << desc.name() << ")-" << desc.id()
                      << ": (" << child.name() << ")-" << child.id() << " with input " << ::util::show (child.activity().input().begin(), child.activity().input().end()));

                  switch (child.execute (exec_policy))
                  {
                  case policy::exec_policy::EXTRACT:
                    insert_activity(child);
                    post_execute_notification (child.id());
                    break;
                  case policy::exec_policy::INJECT:
                    child.finished();
                    DLOG(INFO, "extractor[" << rank << "]: finished (" << child.name() << ")-" << child.id() << ": "
                        << ::util::show (child.activity().output().begin(), child.activity().output().end()));
                    desc.inject (child);
                    break;
                  case policy::exec_policy::EXTERNAL:
                    insert_activity (child);
                    execute_externally (child.id());
                    break;
                  default:
                    LOG(FATAL, "extractor[" << rank << "] got strange classification for activity (" << child.name() << ")-" << child.id());
                    throw std::runtime_error ("invalid classification during execution of activity: " + ::util::show (child));
                  }
                }

                DLOG(TRACE, "done extracting");

                if (desc.is_done ())
                {
                  DLOG(DEBUG, "activity (" << desc.name() << ")-" << active_id << " is done");
                  active_nets_[rank].erase (active_id);
                  post_finished_notification (active_id);
                }
              }
              break;
            case policy::exec_policy::INJECT:
              desc.finished();

              DLOG(INFO, "extractor[" << rank << "]: finished (" << desc.name() << ")-" << desc.id() << ": "
                  << ::util::show (desc.activity().output().begin(), desc.activity().output().end()));

              if (desc.has_parent ())
              {
                lookup (desc.parent()).inject (desc);
                DLOG(INFO, "injected (" << desc.name() << ")-" << desc.id()
                    << " into (" << lookup(desc.parent()).name() << ")-" << desc.parent()
                    << ": " << ::util::show(desc.activity().output().begin(), desc.activity().output().end()));
                DLOG(TRACE, "parent := " << ::util::show(lookup (desc.parent()).activity().transition()));

                post_activity_notification (desc.parent());
              }
              else if (desc.came_from_external())
              {
                DLOG(INFO, "finished (" << desc.name() << ")-" << desc.id() << " external-id := " << desc.from_external_id());
                ext_finished (desc.from_external_id(), policy::codec::encode (desc.activity()));
              }
              else
              {
                throw std::runtime_error ("extractor does not know how to handle this: " + ::util::show (desc));
              }

              remove_activity (desc);

              break;
            case policy::exec_policy::EXTERNAL:
              DLOG(TRACE, "executing externally");
              execute_externally (desc.id());
              break;
            default:
              LOG(FATAL, "extractor[" << rank << "] got strange classification for activity (" << desc.name() << ")-" << desc.id());
              throw std::runtime_error ("extractor got strange classification for activity");
            }
          }
          catch (const activity_not_found<internal_id_type> & ex)
          {
            LOG(WARN, "extractor-" << rank << ": activity could not be found: " << ex.id);
          }
        }
        DLOG(INFO, "extractor-" << rank << " thread stopped...");
      }

      inline
      bool is_valid (const internal_id_type & id) const
      {
        lock_t lock (mutex_);
        return activities_.find(id) != activities_.end();
      }

      void injector(const std::size_t rank)
      {
        DLOG(INFO, "injector[" << rank << "] thread started...");
        for (;;)
        {
          const internal_id_type act_id = inj_q_[rank].get();

          if ( ! is_valid (act_id))
          {
            LOG(WARN, "injecting for invalid id " << act_id);
            continue;
          }

          descriptor_type & desc = lookup (act_id);
          desc.finished();

          sig_finished (this, act_id, policy::codec::encode(desc.activity()));

          DLOG(INFO, "injector[" << rank << "]: finished (" << desc.name() << ")-" << desc.id() << ": "
              << ::util::show (desc.activity().output().begin(), desc.activity().output().end()));

          if (desc.has_parent())
          {
            lookup (desc.parent()).inject (desc);
            DLOG(INFO, "injected (" << desc.name() << ")-" << act_id
                << " into (" << lookup(desc.parent()).name() << ")-" << desc.parent()
                << ": " << ::util::show(desc.activity().output().begin(), desc.activity().output().end()));
            DLOG(TRACE, "parent := " << ::util::show(lookup (desc.parent()).activity().transition()));
              post_activity_notification (desc.parent());
          }
          else if (desc.came_from_external())
          {
            DLOG(INFO, "finished (" << desc.name() << ")-" << act_id << " external-id := " << desc.from_external_id());
            ext_finished (desc.from_external_id(), policy::codec::encode (desc.activity()));
          }
          else
          {
            throw std::runtime_error ("injector does not know how to handle this: " + ::util::show (desc));
          }

          remove_activity (desc);
        }
        DLOG(INFO, "injector-" << rank << " thread stopped...");
      }

      /** Member variables **/
    private:
      boost::function<external_id_type()> external_id_gen_;
      boost::function<internal_id_type()> internal_id_gen_;
      mutable boost::recursive_mutex mutex_;
      mutable boost::recursive_mutex id_gen_mutex_;
      activities_t activities_;

      cmd_q_t cmd_q_;
      active_nets_t *active_nets_;
      active_nets_t *inj_q_;

      external_to_internal_map_t ext_to_int_;

      boost::thread manager_;
      thread_list_t extractor_;
      thread_list_t injector_;

      external_id_type generate_external_id (void) const
      {
        boost::unique_lock<boost::recursive_mutex> lock (id_gen_mutex_);
        return external_id_gen_();
      }

      internal_id_type generate_internal_id (void) const
      {
        boost::unique_lock<boost::recursive_mutex> lock (id_gen_mutex_);
        return internal_id_gen_();
      }

      void activity_needs_attention(const cmd_t & cmd)
      {
        active_nets_.put(cmd.dat);
      }

      void activity_finished(const cmd_t & cmd)
      {
        const internal_id_type internal_id (cmd.dat);

        try
        {
          descriptor_type & desc (lookup(internal_id));
          desc.finished();

          sig_finished (this, internal_id, policy::codec::encode(desc.activity()));

          DLOG(INFO, "finished (" << desc.name() << ")-" << desc.id() << ": "
              << ::util::show (desc.activity().output().begin(), desc.activity().output().end()));

          if (desc.has_parent ())
          {
            post_inject_activity_results (internal_id);
          }
          else if (desc.came_from_external ())
          {
            ext_finished ( desc.from_external_id()
                         , policy::codec::encode (desc.activity())
                         );
            remove_activity (desc);
          }
          else
          {
            LOG(ERROR, "cannot do anything with this activity: " << desc);
            remove_activity (desc);
          }
        }
        catch (const activity_not_found<internal_id_type> &)
        {
          LOG(WARN, "got finished notification for old activity: " << internal_id);
        }
      }

      void activity_failed(const cmd_t & cmd)
      {
        const internal_id_type internal_id (cmd.dat);
        try
        {
          descriptor_type & desc (lookup(internal_id));
          desc.failed();

          DLOG(WARN, "failed (" << desc.name() << ")-" << desc.id());
          sig_failed (this, internal_id, policy::codec::encode(desc.activity()));

          if (desc.has_parent ())
          {
            lookup (desc.parent()).inject (desc);
            post_cancel_activity_notification (desc.parent());
          }
          else if (desc.came_from_external ())
          {
            ext_failed ( desc.from_external_id()
                       , policy::codec::encode (desc.activity())
                       );
          }

          remove_activity (desc);
        }
        catch (const activity_not_found<internal_id_type> &)
        {
          std::cerr << "W: got finished notification for old activity: " << internal_id << std::endl;
        }
      }

      void activity_cancelled(const cmd_t & cmd)
      {
        const internal_id_type internal_id (cmd.dat);
        descriptor_type & desc (lookup(internal_id));

        DLOG(INFO, "cancelled (" << desc.name() << ")-" << desc.id());
        if (desc.has_parent ())
        {
          descriptor_type & parent (lookup (desc.parent()));
          // TODO: should just be remove, not inject
          parent.inject (desc);

          if (! parent.has_children ())
          {
            post_cancelled_notification (parent.id());
          }
        }
        else if (desc.came_from_external ())
        {
          ext_cancelled (desc.from_external_id());
        }

        sig_cancelled ( this
                      , internal_id
                      , policy::codec::encode(desc.activity())
                      );

        remove_activity (desc);
      }

      void cancel_activity(const cmd_t & cmd)
      {
        const internal_id_type internal_id (cmd.dat);
        descriptor_type & desc (lookup(internal_id));

        desc.cancel
            ( boost::bind ( &this_type::post_cancel_activity_notification
                          , this
                          , _1
                          )
            );

        if (desc.sent_to_external())
        {
          ext_cancel ( desc.to_external_id(), "NO REASON GIVEN" );
        }
      }

      void suspend_activity(const cmd_t & cmd)
      {
        lookup(cmd.dat).suspend();
        sig_suspended (this, cmd.dat);
      }

      void resume_activity(const cmd_t & cmd)
      {
        lookup(cmd.dat).resume();
        active_nets_.put (cmd.dat);
        sig_resumed (this, cmd.dat);
      }

      inline void insert_activity(const descriptor_type & desc)
      {
        lock_t lock (mutex_);
        DLOG(TRACE, "inserting activity " << desc.id());

        activities_.insert(std::make_pair(desc.id(), desc));
        if (desc.came_from_external())
        {
          add_map_to_internal (desc.from_external_id(), desc.id());
        }
      }

      inline void remove_activity_by_id(const cmd_t & cmd)
      {
        try
        {
          remove_activity (lookup (cmd.dat));
        }
        catch (std::exception const &ex)
        {
          LOG(WARN, "could not remove activity-" << cmd.dat << ": " << ex.what());
        }
      }

      inline void remove_activity(const descriptor_type & desc)
      {
        lock_t lock (mutex_);
        DLOG(TRACE, "removing activity " << desc.id());

        if (desc.has_children())
          throw std::runtime_error("cannot remove non-leaf: " + ::util::show (desc));
        if (desc.sent_to_external())
        {
          del_map_to_internal (desc.to_external_id(), desc.id());
        }
        if (desc.came_from_external())
        {
          del_map_to_internal (desc.from_external_id(), desc.id());
        }

        activities_.erase (desc.id());
      }

      inline descriptor_type & lookup(const internal_id_type & id)
      {
        lock_t (mutex_);
        typename activities_t::iterator a = activities_.find(id);
        if (a == activities_.end()) throw activity_not_found<internal_id_type>("lookup("+::util::show(id)+") failed!", id);
        return a->second;
      }

      inline const descriptor_type & lookup(const internal_id_type & id) const
      {
        lock_t (mutex_);
        typename activities_t::const_iterator a = activities_.find(id);
        if (a == activities_.end()) throw activity_not_found<internal_id_type>("lookup("+::util::show(id)+") failed!", id);
        return a->second;
      }

      friend class boost::serialization::access;
      template<class Archive>
      void serialize (Archive & ar, const unsigned int)
      {
      }
    };
  }
}

#endif
