// alexander.petry@itwm.fraunhofer.de

#ifndef WE_MGMT_LAYER_HPP
#define WE_MGMT_LAYER_HPP 1

#include <fhglog/fhglog.hpp>

#include <fhg/assert.hpp>
#include <fhg/error_codes.hpp>
#include <fhg/util/show.hpp>
#include <fhg/util/warnings.hpp>
#include <fhg/util/threadname.hpp>

#include <we/type/net.hpp>

#include <boost/random.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <we/mgmt/exception.hpp>
#include <we/mgmt/bits/commands.hpp>
#include <we/mgmt/bits/queue.hpp>
#include <we/mgmt/bits/set.hpp>
#include <we/mgmt/bits/signal.hpp>
#include <we/mgmt/bits/descriptor.hpp>
#include <we/mgmt/bits/execution_policy.hpp>

#include <we/type/id.hpp>
#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>
#include <we/mgmt/type/activity.hpp>

#include <boost/foreach.hpp>

namespace we { namespace mgmt {
    class layer
    {
    public:
      typedef std::string id_type;

      typedef std::string external_id_type;
      typedef petri_net::activity_id_type internal_id_type;

      typedef std::string encoded_type;
      typedef std::string reason_type;
      typedef std::string result_type;
      typedef std::string status_type;

    private:
      typedef boost::unordered_map<external_id_type, internal_id_type> external_to_internal_map_t;

      typedef std::vector<boost::thread *> thread_list_t;

      typedef boost::shared_ptr<detail::descriptor> descriptor_ptr;
      typedef boost::unordered_map<internal_id_type, descriptor_ptr> activities_t;

      // manager thread
      typedef detail::commands::command_t<detail::commands::CMD_ID, internal_id_type> cmd_t;
      typedef detail::queue<cmd_t, 0> cmd_q_t;

      // extractor
      //! \todo is it necessary to use a locked data structure?
      typedef detail::set<internal_id_type, 0> active_nets_t;

      typedef boost::unique_lock<boost::recursive_mutex> lock_t;
    public:

      /******************************
       * EXTERNAL API
       *****************************/

      // observe
      util::signal<void (const layer *, internal_id_type const &)> sig_submitted;
      util::signal<void (const layer *, internal_id_type const &, std::string const &)> sig_finished;
      util::signal<void (const layer *, internal_id_type const &, std::string const &)> sig_failed;
      util::signal<void (const layer *, internal_id_type const &, std::string const &)> sig_canceled;
      util::signal<void (const layer *, internal_id_type const &)> sig_executing;

      /**
       * Submit a new petri net to the petri-net management layer
       *
       *          pre-conditions: none
       *
       *          side-effects: parses the passed data
       *                                        registeres the petri-net with the mgmt layer
       *
       *          post-conditions: the net is registered is with id "id"
       *
       */
      void submit (const external_id_type&, const encoded_type&, const we::type::user_data &);
      void submit (const external_id_type&, const we::mgmt::type::activity_t&, const we::type::user_data &);

      /**
       * Inform the management layer to cancel a previously submitted net
       *
       *          pre-conditions:
       *                - a  network  must have  been registered  and
       *                  assigned the id "id"
       *
       *          side-effects:
       *                -  the  hierarchy   belonging  to  the  net  is
       *                   canceled in turn
       *
       *          post-conditions:
       *
       *                - the  internal   state  of  the   network  switches  to
       *                  CANCELING
       *                - all children of the network will be terminated
       */
      bool cancel (const external_id_type&, const reason_type&);

      /**
       * Inform the management  layer that an execution finished  with the given
       * result
       *
       *          pre-conditions:
       *
       *              - the  management  layer   submitted  an  activity  to  be
       *                executed with id "id"
       *
       *          side-effects:
       *
       *              - the results are integrated into the referenced net
       *
       *          post-conditions:
       *
       *               - the node belonging to this is activity is removed
       **/
      bool finished (const external_id_type&, const result_type&);

      /**
       * Inform the  management layer  that an execution  failed with  the given
       * result
       *
       *          pre-conditions:
       *
       *               - the  management  layer  submitted  an  activity  to  be
       *                 executed with id "id"
       *
       *          side-effects:
       *
       *               - the results are integrated into the referenced net
       *
       *          post-conditions:
       *
       *                - the node belonging to this activity is removed
       **/
      bool failed ( const external_id_type&
                  , const result_type&
                  , const int error_code
                  , const std::string&
                  );

      /**
       * Inform the management layer that an execution has been canceled
       *
       *          pre-conditions:
       *                  - the management layer submitted an activity to be executed with id "id"
       *                  - the management layer requested the cancellation of an activity
       *
       *          side-effects:
       *                  - the enclosing workflow will be informed that an activity has been canceled
       *
       *          post-conditions:
       *                  - the node belonging to this activity is removed
       **/
      bool canceled (const external_id_type&);

      // END: EXTERNAL API

      status_type status(const external_id_type & id)
      {
        return fhg::util::show (*lookup (map_to_internal(id)));
      }

      void print_statistics (std::ostream & s) const
      {
        s << "==== begin layer statistics ====" << std::endl;

        lock_t lock (mutex_);
        s << "   #activities := " << activities_.size() << std::endl;
        s << "   ext -> int := " << fhg::util::show (ext_to_int_.begin(), ext_to_int_.end());
        s << std::endl;
        s << std::endl;

        std::vector <internal_id_type> ids;
        ids.reserve (activities_.size());

        for ( activities_t::const_iterator desc (activities_.begin())
            ; desc != activities_.end()
            ; ++desc
            )
        {
          ids.push_back (desc->first);
        }

        std::sort (ids.begin(), ids.end());

        for ( std::vector<internal_id_type>::const_iterator id (ids.begin())
            ; id != ids.end()
            ; ++id
            )
        {
          s << *activities_.at(*id);
          s << std::endl;
        }

        s << std::endl;
        s << "==== end layer statistics ====" << std::endl;
      }
    private:
      // handle execution layer
      boost::function<void ( external_id_type const &
                           , encoded_type const &
                           , const std::list<we::type::requirement_t>&
                           , const we::type::schedule_data&
                           , const we::type::user_data &
                           )> ext_submit;
      boost::function<bool ( external_id_type const &
                           , reason_type const &
                           )>  ext_cancel;
      boost::function<bool ( external_id_type const &
                           , result_type const &
                           )>  ext_finished;
      boost::function<bool ( external_id_type const &
                           , result_type const &
                           , const int error_code
                           , std::string const & reason
                           )>  ext_failed;
      boost::function<bool (external_id_type const &)>                       ext_canceled;

      void submit (const descriptor_ptr & desc)
      {
        insert_activity(desc);

        sig_submitted (this, desc->id());
        post_execute_notification (desc->id());
      }

      void add_map_to_internal ( const external_id_type & external_id
                               , const internal_id_type & internal_id
                               )
      {
        lock_t lock (mutex_);
        external_to_internal_map_t::const_iterator mapping
          (ext_to_int_.find(external_id));

        if (mapping != ext_to_int_.end())
        {
          throw exception::already_there
            ( "already_there: ext_id := "
            + fhg::util::show(external_id)
            + " -> int_id := "
            + fhg::util::show(mapping->second)
            , external_id
            );
        }

        ext_to_int_.insert ( external_to_internal_map_t::value_type
                           (external_id, internal_id)
                           );
        DLOG(TRACE, "added mapping " << external_id << " -> " << internal_id);
      }

      void del_map_to_internal ( const external_id_type & external_id
                               , const internal_id_type & internal_id
                               )
      {
        lock_t lock (mutex_);

        external_to_internal_map_t::const_iterator mapping
          (ext_to_int_.find(external_id));
        if (  mapping == ext_to_int_.end()
           || mapping->second != internal_id
           )
        {
          throw exception::no_such_mapping
            ( "no_such_mapping: ext := "
            + fhg::util::show(external_id)
            + " -> int := "
            + fhg::util::show(internal_id)
            , external_id
            );
        }

        ext_to_int_.erase (external_id);

        DLOG(TRACE, "deleted mapping " << external_id << " -> " << internal_id);
      }

      external_to_internal_map_t::mapped_type map_to_internal ( const external_id_type & external_id ) const
      {
        lock_t lock (mutex_);

        external_to_internal_map_t::const_iterator mapping (ext_to_int_.find(external_id));
        if (mapping != ext_to_int_.end())
        {
          return mapping->second;
        }
        else
        {
          throw exception::no_such_mapping ("no_such_mapping: ext_id := " + fhg::util::show (external_id), external_id);
        }
      }
    public:
      /**
       * Constructor calls
       */
      layer()
        : sig_submitted("sig_submitted")
        , sig_finished("sig_finished")
        , sig_failed("sig_failed")
        , sig_canceled("sig_canceled")
        , sig_executing("sig_executing")
        , internal_id_gen_(&petri_net::activity_id_generate)
      {
        start();
      }

      template <class E, typename G>
      layer(E * exec_layer, G gen)
        : sig_submitted("sig_submitted")
        , sig_finished("sig_finished")
        , sig_failed("sig_failed")
        , sig_canceled("sig_canceled")
        , sig_executing("sig_executing")
        , external_id_gen_(gen)
        , internal_id_gen_(&petri_net::activity_id_generate)
      {
        ext_submit = (boost::bind (& E::submit, exec_layer, _1, _2, _3, _4, _5));
        ext_cancel = (boost::bind (& E::cancel, exec_layer, _1, _2));
        ext_finished = (boost::bind (& E::finished, exec_layer, _1, _2));
        ext_failed = (boost::bind (& E::failed, exec_layer, _1, _2, _3, _4));
        ext_canceled = (boost::bind (& E::canceled, exec_layer, _1));

        start();
      }

      void set_id_generator (boost::function<external_id_type()> gen)
      {
        lock_t gen_lock (id_gen_mutex_);

        external_id_gen_ = gen;
      }

      ~layer()
      {
        // stop threads
        stop();

        // cancel external activities

        {
          lock_t lock (mutex_);
          // clean up all activities
          while (! activities_.empty())
          {
            DLOG(WARN, "removing remaining activity: " << activities_.begin()->second);
            activities_.erase (activities_.begin());
          }
          ext_to_int_.clear();
        }
      }

      /* internal functions */
    private:
      void start()
      {
        lock_t lock (mutex_);

        manager_   = boost::thread(boost::bind(&layer::manager, this));
        fhg::util::set_threadname (manager_, "[we-mgr]");

        active_nets_ = new active_nets_t;
        start_threads ( "we-extract"
                      , extractor_
                      , boost::bind (&layer::extractor, this)
                      );

        inj_q_ = new active_nets_t;
        start_threads ( "we-inject"
                      , injector_
                      , boost::bind (&layer::injector, this)
                      );
      }

      void start_threads ( std::string const & tag
                         , thread_list_t& list
                         , boost::function<void ()> tf
                         )
      {
        std::stringstream sstr;
        sstr << "[" << tag << "]";
        boost::thread *thrd (new boost::thread (tf));
        fhg::util::set_threadname (*thrd, sstr.str());
        list.push_back (thrd);
      }

      void stop_threads (thread_list_t& list)
      {
        BOOST_FOREACH (boost::thread* t, list)
        {
          t->interrupt();
          t->join();
          delete t;
        }
        list.clear();
      }

      void stop()
      {
        DMLOG(TRACE, "Workflow Management layer stopping...");

        DLOG(TRACE, "cleaning up manager thread...");
        manager_.interrupt();
        manager_.join();
        DLOG(TRACE, "done.");

        DLOG(TRACE, "cleaning up injector threads...");
        stop_threads (injector_);
        DLOG(TRACE, "done.");

        DLOG(TRACE, "cleaning up extractor threads...");
        stop_threads (extractor_);
        DLOG(TRACE, "done.");

        delete active_nets_;
        active_nets_ = NULL;

        delete inj_q_;
        inj_q_ = NULL;
      }

      void manager()
      {
        using namespace we::mgmt::detail::commands;
        DLOG(TRACE, "manager thread started...");
        for (;;)
        {
          cmd_t cmd = cmd_q_.get();
          try
          {
            cmd.handle();
          }
          catch (std::exception const& ex)
          {
            LOG( WARN
               , "error during manager command handling: command: "
               << cmd.name
               << " failed: "
               << ex.what()
               );
          }
        }
        DLOG(TRACE, "manager thread stopped...");
      }

      inline
      void post_activity_notification (const internal_id_type & id)
      {
        if (is_valid(id))
        {
          active_nets_->put(id);
        }
        else
        {
          LOG(ERROR, "id is not valid anymore: " << id);
        }
      }

      inline
      void post_inject_activity_results (const internal_id_type & id)
      {
        if (is_valid(id))
        {
          inj_q_->put ( id );
        }
        else
        {
          LOG(ERROR, "id is not valid anymore: " << id);
        }
      }

      inline
      void post_finished_notification (const internal_id_type & id)
      {
        if (is_valid(id))
        {
          inj_q_->put ( id );
        }
        else
        {
          LOG(ERROR, "id is not valid anymore: " << id);
        }
      }

      inline
      void post_failed_notification (const internal_id_type & id)
      {
        if (is_valid(id))
        {
          cmd_q_.put(make_cmd("activity_failed", id, boost::bind(&layer::activity_failed, this, _1)));
        }
        else
        {
          LOG(ERROR, "id is not valid anymore: " << id);
        }
      }

      inline
      void post_canceled_notification (const internal_id_type & id)
      {
        if (is_valid(id))
        {
          cmd_q_.put(make_cmd("activity_canceled", id, boost::bind(&layer::activity_canceled, this, _1)));
        }
        else
        {
          LOG(ERROR, "id is not valid anymore: " << id);
        }
      }

      inline
      void post_cancel_activity_notification (const internal_id_type & id)
      {
        if (is_valid(id))
        {
          cmd_q_.put (make_cmd("cancel_activity", id, boost::bind(&layer::cancel_activity, this, _1)));
        }
        else
        {
          LOG(ERROR, "id is not valid anymore: " << id);
        }
      }

      inline
      void post_execute_notification (const internal_id_type & id)
      {
        if (is_valid(id))
        {
          post_activity_notification (id);
        }
        else
        {
          LOG(ERROR, "id is not valid anymore: " << id);
        }
      }

      inline
      void execute_externally (const internal_id_type & int_id)
      {
        descriptor_ptr desc (lookup (int_id));

        // create external id
        external_id_type ext_id ( generate_external_id() );
        desc->sent_to_external_as (ext_id);
        add_map_to_internal (ext_id, int_id);

        // copy the activity and let it be handled internally on the next level
        we::mgmt::type::activity_t ext_act (desc->activity());
        ext_act.transition().set_internal(true);

        DLOG(DEBUG, "submitting internal activity " << int_id << " to external with id " << ext_id);

        we::type::schedule_data schedule_data
          ( ext_act.get_schedule_data<long> ("num_worker")
          , ext_act.get_schedule_data<long> ("vmem")
          );

        ext_submit ( ext_id
                   , ext_act.to_string()
                   , ext_act.transition().requirements()
                   , schedule_data
                   , desc->get_user_data ()
                   );
      }

      // WORK HERE: rewrite!
      void extractor()
      {
        DLOG (TRACE, "extractor thread started...");
        for (;;)
        {
          internal_id_type active_id = active_nets_->get();
          if (! is_valid (active_id))
          {
            DLOG( WARN
                , "extractor woken up by old activity id " << active_id
                );
            continue;
          }

          try
          {
            descriptor_ptr desc (lookup(active_id));
            DLOG(TRACE, "extractor puts attention to activity " << active_id);

            if (desc->activity().is_canceling())
            {
              if (!desc->has_children())
              {
                post_canceled_notification (active_id);
              }

              continue;
            }

            // TODO: check status flags
            if (! desc->is_alive ())
            {
              LOG(DEBUG, "activity (" << desc->name() << ")-" << active_id << " is on hold");
              continue;
            }

            sig_executing (this, desc->id());

            try
            {
              // classify/execute
              //    EXTRACT: while loop
              //       classify/execute
              //          EXTRACT: remember id
              //       INJECT: inject
              //       EXTERN: extern
              //    INJECT:  inject to parent / notify client
              //    EXTERN:  send to extern
              do_execute (desc);
            }
            catch (const std::exception & ex)
            {
              LOG( ERROR
                 , "extractor: something went wrong during execution of: "
                 << desc->name() << ": " << ex.what()
                 );
              desc->set_error_code (fhg::error::UNEXPECTED_ERROR);
              desc->set_error_message
                ( std::string ("in: '")
                + desc->name ()
                + std::string ("' ")
                + ex.what ()
                );
              desc->set_result (desc->activity().to_string());
              post_failed_notification (desc->id());
            }
          }
          catch (const exception::activity_not_found& ex)
          {
            LOG(WARN, "extractor: activity could not be found: " << ex.id());
          }
        }
        DLOG(INFO, "extractor thread stopped...");
      }

      inline
      void do_execute (descriptor_ptr desc)
      {
        policy::execution_policy exec_policy;

        switch (desc->execute (&exec_policy))
        {
        case policy::execution_policy::EXTRACT:
          {
            DLOG(TRACE, "extractor extracting from net: " << desc->name());

            while (desc->is_alive () && desc->enabled())
            {
              descriptor_ptr child (new detail::descriptor(desc->extract(generate_internal_id())));
              child->inject_input ();
              child->set_user_data (desc->get_user_data ());

              DLOG (TRACE, "extractor: extracted from (" << desc->name() << ")-" << desc->id()
                  << ": (" << child->name() << ")-" << child->id());

              switch (child->execute (&exec_policy))
              {
              case policy::execution_policy::EXTRACT:
                insert_activity(child);
                post_execute_notification (child->id());
                break;
              case policy::execution_policy::INJECT:
                child->finished();
                DLOG (TRACE, "extractor: finished (" << child->name() << ")-" << child->id());
                desc->inject (*child);
                break;
              case policy::execution_policy::EXTERNAL:
                insert_activity (child);
                execute_externally (child->id());
                break;
              default:
                throw std::runtime_error ("invalid classification during execution of activity: " + fhg::util::show (*child));
              }

              DLOG(TRACE, "extractor done with child: " << child->id());
            }

            DLOG(TRACE, "extractor done extracting (#children = " << desc->child_count() << ")");


            if (desc->is_done ())
            {
              DLOG(DEBUG, "extractor: activity (" << desc->name() << ")-" << desc->id() << " is done");
              active_nets_->erase (desc->id());
              do_inject (desc);
            }
          }
          break;
        case policy::execution_policy::INJECT:
          do_inject (desc);
          break;
        case policy::execution_policy::EXTERNAL:
          DLOG(TRACE, "extractor: executing externally: " << desc->id());
          execute_externally (desc->id());
          break;
        default:
          LOG(FATAL, "extractor: got strange classification for activity (" << desc->name() << ")-" << desc->id());
          throw std::runtime_error ("extractor got strange classification for activity");
        }
      }

      inline
      bool is_valid (const internal_id_type & id) const
      {
        lock_t lock (mutex_);
        return activities_.find(id) != activities_.end();
      }

      void injector()
      {
        DLOG (TRACE, "injector thread started...");
        for (;;)
        {
          const internal_id_type act_id = inj_q_->get();

          if ( ! is_valid (act_id))
          {
            DLOG(WARN, "injector woken up by old activity id " << act_id);
            continue;
          }

          descriptor_ptr desc;
          try
          {
            desc = lookup (act_id);
          }
          catch (std::exception const &ex)
          {
            MLOG (ERROR, "internal error: activity not valid anymore");
            continue;
          }

          try
          {
            do_inject (desc);
          }
          catch (std::exception const & ex)
          {
            LOG(ERROR, "injector got exception during injecting: " << ex.what());

            desc->set_error_code (fhg::error::UNEXPECTED_ERROR);
            desc->set_error_message (ex.what ());
            desc->set_result (desc->activity ().to_string());

            post_failed_notification (desc->id ());
          }
          catch (...)
          {
            LOG(ERROR, "injector got unexpected exception during injecting!");
          }
        }
        DLOG(INFO, "injector thread stopped...");
      }

      void do_inject (descriptor_ptr desc)
      {
        desc->finished();
        DLOG (TRACE, "finished (" << desc->name() << ")-" << desc->id());

        if (desc->has_parent())
        {
          DLOG (TRACE, "injecting (" << desc->name() << ")-" << desc->id()
              << " into (" << lookup(desc->parent())->name() << ")-" << desc->parent()
               );
          lookup (desc->parent())->inject
            ( *desc
            , boost::bind ( &layer::post_activity_notification
                          , this
                          , _1
                          )
            );
        }
        else if (desc->came_from_external())
        {
          if (desc->activity ().is_failed ())
          {
            DLOG ( INFO
                 , "failed (" << desc->name() << ")-" << desc->id()
                 << " external-id := " << desc->from_external_id()
                 );
            ext_failed ( desc->from_external_id()
                       , desc->activity().to_string()
                       , desc->error_code()
                       , desc->error_message()
                       );
          }
          else if (desc->activity ().is_canceled ())
          {
            DLOG ( INFO
                 , "canceled (" << desc->name() << ")-" << desc->id()
                 << " external-id := " << desc->from_external_id()
                 );
            ext_canceled (desc->from_external_id());
          }
          else
          {
            DLOG ( INFO
                 , "finished (" << desc->name() << ")-" << desc->id()
                 << " external-id := " << desc->from_external_id()
                 );
            ext_finished ( desc->from_external_id()
                         , desc->activity().to_string()
                         );
          }
        }
        else
        {
          throw std::runtime_error ("STRANGE! cannot inject: " + fhg::util::show (*desc));
        }

        if (sig_finished.connected())
          sig_finished ( this
                       , desc->id()
                       , desc->activity().to_string()
                       );
        remove_activity (desc);
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

      void activity_failed(const cmd_t & cmd)
      {
        const internal_id_type internal_id (cmd.dat);
        try
        {
          descriptor_ptr desc (lookup(internal_id));
          desc->failed();

          DMLOG ( WARN
                , "failed (" << desc->name() << ")-" << desc->id() << " : "
                << desc->error_message ()
                );

          if (sig_failed.connected())
            sig_failed ( this
                       , internal_id
                       , desc->activity().to_string()
                       );

          if (desc->has_parent ())
          {
            descriptor_ptr parent_desc =
              lookup (desc->parent());

            parent_desc->child_failed( *desc
                                     , desc->error_code()
                                     , desc->error_message()
                                     );
            // FIXME: handle failure in a meaningful way:
            //     - check failure reason
            //         - EAGAIN reschedule (had not been submitted yet)
            //         - ECRASH activity crashed (idempotence criteria)
            post_cancel_activity_notification (parent_desc->id ());
          }
          else if (desc->came_from_external ())
          {
            ext_failed ( desc->from_external_id()
                       , desc->activity().to_string()
                       , desc->error_code()
                       , desc->error_message()
                       );
          }
          else
          {
            throw std::runtime_error ("activity failed, but I don't know what to do with it: " + fhg::util::show (desc));
          }

          remove_activity (desc);
        }
        catch (const exception::activity_not_found&)
        {
          LOG(WARN, "got failed notification for old activity: " << internal_id);
        }
      }

      void activity_canceled(const cmd_t & cmd)
      {
        const internal_id_type internal_id (cmd.dat);
        try
        {
          descriptor_ptr desc (lookup(internal_id));
          desc->canceled();

          if (desc->has_parent ())
          {
            DLOG ( INFO, "activity "
                 << desc->name ()
                 << " canceled and has a parent: reason := "
                 << desc->error_message ()
                 );

            descriptor_ptr parent (lookup (desc->parent()));
            parent->child_canceled(*desc, "TODO: child canceled reason");

            if (! parent->has_children ())
            {
              post_canceled_notification (parent->id());
            }
            else
            {
              post_cancel_activity_notification (parent->id ());
            }
          }
          else if (desc->came_from_external ())
          {
            DMLOG (TRACE, "notifying agent: failed (" << desc->name() << ")-" << desc->id());

            ext_failed ( desc->from_external_id()
                       , desc->activity().to_string()
                       , desc->error_code()
                       , desc->error_message()
                       );
          }
          else
          {
            throw std::runtime_error ("activity canceled, but I don't know what to do with it: " + fhg::util::show (*desc));
          }

          if (sig_canceled.connected())
            sig_canceled ( this
                          , internal_id
                          , desc->activity().to_string()
                          );

          remove_activity (desc);
        }
        catch (const exception::activity_not_found&)
        {
          DLOG(WARN, "got canceled notification for old activity: " << internal_id);
        }
      }

      void cancel_activity(const cmd_t & cmd)
      {
        const internal_id_type internal_id (cmd.dat);
        try
        {
          descriptor_ptr desc (lookup(internal_id));

          if (desc->has_children())
          {
            desc->cancel
              ( boost::bind ( &layer::post_cancel_activity_notification
                            , this
                            , _1
                            )
              );
          }
          else if (desc->sent_to_external())
          {
            if (! (  desc->activity().is_canceling()
                  || desc->activity().is_failed()
                  || desc->activity().is_canceled()
                  || desc->activity().is_finished()
                  )
               )
            {
              DMLOG ( TRACE
                    , "canceling external activity " << desc->to_external_id ()
                    << " name: " << desc->name ()
                    << " reason: " << desc->error_message ()
                    );

              ext_cancel ( desc->to_external_id()
                         , "WFE policy cancel-on-failure in place"
                         );
            }
          }
          else
          {
            post_canceled_notification (desc->id());
          }
        }
        catch (const exception::activity_not_found&)
        {
          DMLOG (WARN, "got cancel request for old activity: " << internal_id);
        }
      }

      inline void insert_activity(const descriptor_ptr & desc)
      {
        lock_t lock (mutex_);
        DLOG(TRACE, "inserting activity " << desc->id());

        activities_.insert(std::make_pair(desc->id(), desc));
        if (desc->came_from_external())
        {
          add_map_to_internal (desc->from_external_id(), desc->id());
        }
      }

      inline void remove_activity(const descriptor_ptr & desc)
      {
        lock_t lock (mutex_);
        DLOG(TRACE, "removing activity " << desc->id());

        if (desc->has_children())
          throw std::runtime_error("cannot remove non-leaf: " + fhg::util::show (desc));
        if (desc->sent_to_external())
        {
          del_map_to_internal (desc->to_external_id(), desc->id());
        }
        if (desc->came_from_external())
        {
          del_map_to_internal (desc->from_external_id(), desc->id());
        }

        activities_.erase (desc->id());
      }

      inline descriptor_ptr lookup (const internal_id_type& id)
      {
        lock_t (mutex_);

        activities_t::iterator a (activities_.find(id));

        if (a == activities_.end())
          {
            throw exception::activity_not_found
              ("lookup (" + fhg::util::show (id) + ") failed!", id);
          }

        return a->second;
      }
    };
  }
}

#endif
