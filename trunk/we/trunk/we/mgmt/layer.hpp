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

      typedef typename boost::unordered_map<internal_id_type, activity_type> activities_t;
      typedef typename activity_type::output_t output_type;
    private:
      typedef detail::set<internal_id_type> active_nets_t;

      typedef boost::unordered_set<internal_id_type> child_set_t;
      typedef boost::unordered_map<internal_id_type, internal_id_type> child_to_parent_map_t;
      typedef boost::unordered_map<internal_id_type, child_set_t> parent_to_children_map_t;

      typedef boost::unordered_map<external_id_type, internal_id_type> external_to_internal_map_t;
      typedef boost::unordered_map<internal_id_type, external_id_type> internal_to_external_map_t;

      // manager thread
      typedef detail::commands::command_t<detail::commands::CMD_ID, internal_id_type> cmd_t;
      typedef detail::queue<cmd_t> cmd_q_t;

      // injector thread
      typedef internal_id_type inj_cmd_t;
      typedef detail::queue<inj_cmd_t> inj_q_t;

      typedef internal_id_type executor_cmd_t;
      typedef detail::queue<executor_cmd_t> exec_q_t;

    public:

      /******************************
       * EXTERNAL API
       *****************************/

      // observe
      util::signal<void (const this_type *, internal_id_type const &, std::string const &)> sig_submitted;
      util::signal<void (const this_type *, internal_id_type const &, std::string const &)> sig_finished;
      util::signal<void (const this_type *, internal_id_type const &, std::string const &)> sig_failed;
      util::signal<void (const this_type *, internal_id_type const &, std::string const &)> sig_cancelled;
      util::signal<void (const this_type *, internal_id_type const &, std::string const &)> sig_executing;

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
        const internal_id_type internal_id ( add_external_id (id) );
        activity_type act = policy::codec::decode(bytes);
        submit (internal_id, act);
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
        we::util::remove_unused_variable_warning(id);
        we::util::remove_unused_variable_warning(reason);
        // TODO: traverse children with a depth first search
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
        internal_id_type internal_id ( map_to_internal (id) );

        activity_type res_act ( policy::codec::decode (result) );
        res_act.collect_output();

        lookup (internal_id).output().swap (res_act.output());
        inj_q_.put ( internal_id );
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
      bool failed(const external_id_type & id, const result_type & result) throw()
      {
        internal_id_type internal_id ( map_to_internal (id) );

        activity_type res_act ( policy::codec::decode (result) );
        res_act.collect_output();

        lookup (internal_id).output().swap (res_act.output());
        inj_q_.put ( internal_id );
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
        we::util::remove_unused_variable_warning(id);
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
        post_suspend_activity_notification( map_to_internal(id) );
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
        post_resume_activity_notification( map_to_internal(id) );
        return true;
      }

      // END: EXTERNAL API

      status_type status(const external_id_type & id) throw (std::exception)
      {
        we::util::remove_unused_variable_warning(id);
        throw std::runtime_error("not yet implemented: status");
      }

    private:
      // handle execution layer
      util::signal<void (external_id_type const &, encoded_type const &)> ext_submit;
      util::signal<bool (external_id_type const &, reason_type const &)>  ext_cancel;
      util::signal<bool (external_id_type const &, result_type const &)>  ext_finished;
      util::signal<bool (external_id_type const &, result_type const &)>  ext_failed;
      util::signal<bool (external_id_type const &)>                       ext_cancelled;

      void submit (const internal_id_type & id, activity_type & act)
      {
        std::cerr << "D: pre-submit act["<< id << "]" << std::endl;
        {
          try
          {
            policy::validator::validate (act);
            insert_activity(id, act);
            std::cerr << "D: submitted act["<< id << "]" << std::endl;
            sig_submitted (this, id, policy::codec::encode (act));

            post_execute_notification (id);
          }
          catch (const std::exception & ex)
          {
            throw exception::validation_error( std::string("layer::submit(): invalid activity:")
                                               + " id := " + ::util::show(id)
                                               + " name := " + act.transition().name()
                                             , act.transition().name()
                                             );
          }
        }
      }

      internal_id_type add_external_id (const external_id_type & external_id)
      {
        internal_id_type internal_id ( internal_id_gen_() );
        add_external_to_internal_mapping (external_id, internal_id);
        return internal_id;
      }

      void add_external_to_internal_mapping ( const external_id_type & external_id
                                            , const internal_id_type & internal_id
                                            )
      {
        boost::unique_lock<boost::shared_mutex> lock (id_map_mutex_);

        typename external_to_internal_map_t::const_iterator mapping (ex_to_in_.find(external_id));
        if (mapping != ex_to_in_.end())
        {
          throw exception::already_there<external_id_type> ("already_there: ext_id := " + ::util::show(external_id) + " -> int_id := " + ::util::show(mapping->second), external_id);
        }
        ex_to_in_.insert ( typename external_to_internal_map_t::value_type (external_id, internal_id) );
        in_to_ex_.insert ( typename internal_to_external_map_t::value_type (internal_id, external_id) );
      }

      void remove_external_to_internal_mapping ( const external_id_type & external_id
                                               , const internal_id_type & internal_id
                                               )
      {
        boost::unique_lock<boost::shared_mutex> lock (id_map_mutex_);
        ex_to_in_.erase (ex_to_in_.find (external_id));
        in_to_ex_.erase (in_to_ex_.find (internal_id));
      }

      typename external_to_internal_map_t::mapped_type map_to_internal (const external_id_type & external_id) const
      {
        boost::shared_lock<boost::shared_mutex> lock (id_map_mutex_);

        typename external_to_internal_map_t::const_iterator mapping (ex_to_in_.find(external_id));
        if (mapping != ex_to_in_.end())
        {
          return mapping->second;
        }
        else
        {
          throw exception::no_such_mapping<external_id_type> ("no_such_mapping: ext_id := " + ::util::show (external_id), external_id);
        }
      }

      typename internal_to_external_map_t::mapped_type map_to_external (const internal_id_type & internal_id) const
      {
        boost::shared_lock<boost::shared_mutex> lock (id_map_mutex_);

        typename internal_to_external_map_t::const_iterator mapping (in_to_ex_.find(internal_id));
        if (mapping != in_to_ex_.end())
        {
          return mapping->second;
        }
        else
        {
          throw exception::no_such_mapping<internal_id_type> ("no_such_mapping: int_id := " + ::util::show (internal_id), internal_id);
        }
      }

    public:
      /**
       * Constructor calls
       */
      layer()
        : internal_id_gen_(&internal_id_traits::generate)
        , barrier_(4 + 1) // 1 + injector, manager, executor, extractor
        , cmd_q_(policy::max_command_queue_size())
        , active_nets_(policy::max_active_nets())
        , inj_q_(policy::max_injector_queue_size())
        , exec_q_(policy::max_executor_queue_size())
      {
        start();
      }

      template <class E>
      explicit
      layer(E * exec_layer)
        : internal_id_gen_(&internal_id_traits::generate)
        , barrier_(4 + 1) // 1 + injector, manager, executor, extractor
        , cmd_q_(policy::max_command_queue_size())
        , active_nets_(policy::max_active_nets())
        , inj_q_(policy::max_injector_queue_size())
        , exec_q_(policy::max_executor_queue_size())
      {
        connect (exec_layer);
        start();
      }

      template <class E, typename G>
      layer(E * exec_layer, G gen)
        : external_id_gen_(gen)
        , internal_id_gen_(&internal_id_traits::generate)
        , barrier_(4 + 1) // 1 + injector, manager, executor, extractor
        , cmd_q_(policy::max_command_queue_size())
        , active_nets_(policy::max_active_nets())
        , inj_q_(policy::max_injector_queue_size())
        , exec_q_(policy::max_executor_queue_size())
      {
        connect (exec_layer);
        start();
      }

      template <typename T>
      void connect ( T * t )
      {
        disconnect ();

        // for now extract the functions from a given class
        ext_submit.connect (boost::bind (& T::submit, t, _1, _2));
        ext_cancel.connect (boost::bind (& T::cancel, t, _1, _2));
        ext_finished.connect (boost::bind (& T::finished, t, _1, _2));
        ext_failed.connect (boost::bind (& T::failed, t, _1, _2));
        ext_cancelled.connect (boost::bind (& T::cancelled, t, _1));
      }

      void disconnect ()
      {
        ext_submit.clear();
        ext_cancel.clear();
        ext_finished.clear();
        ext_failed.clear();
        ext_cancelled.clear();
      }

      template <typename IdGen>
      void set_id_generator ( IdGen gen )
      {
        external_id_gen_ = gen;
      }

      ~layer()
      {
        stop();

        // stop threads
        //
        // cancel activities
        //
        // delete memory allocations
        //
        // clean up all activity
        for (typename activities_t::iterator a = (activities_.begin()); a != activities_.end(); ++a)
        {
          std::cerr << "D: removing act[" << a->first << "]" << std::endl;
        }
      }

      void print_statistics (std::ostream & s) const
      {
        s << "==== begin layer statistics ====" << std::endl;
        s << "   #activities := " << activities_.size() << std::endl;
        s << "   ext <-> int := [";
        for ( typename external_to_internal_map_t::const_iterator e_to_i (ex_to_in_.begin())
            ; e_to_i != ex_to_in_.end()
            ; ++e_to_i
            )
        {
          if (e_to_i != ex_to_in_.begin())
          {
            s << ", ";
          }
          s << e_to_i->first << " == " << e_to_i->second;
        }
        s << "]";
        s << std::endl;
        s << std::endl;

        for ( typename activities_t::const_iterator act (activities_.begin())
            ; act != activities_.end()
            ; ++act
            )
        {
          print_activity_info (s, act->second);
        }

        s << "==== end layer statistics ====" << std::endl;
      }

      /* internal functions */
    private:
      void start()
      {
        manager_   = boost::thread(boost::bind(&this_type::manager, this));
        extractor_ = boost::thread(boost::bind(&this_type::extractor, this));
        injector_  = boost::thread(boost::bind(&this_type::injector, this));
        executor_  = boost::thread(boost::bind(&this_type::executor, this));
        barrier_.wait();
      }

      void stop()
      {
        std::cerr << "D: cleaning up executor thread..." << std::endl;
        executor_.interrupt();
        executor_.join();

        std::cerr << "D: cleaning up injector thread..." << std::endl;
        injector_.interrupt();
        injector_.join();

        std::cerr << "D: cleaning up manager thread..." << std::endl;
        manager_.interrupt();
        manager_.join();

        std::cerr << "D: cleaning up extractor thread..." << std::endl;
        extractor_.interrupt();
        extractor_.join();

      }

      void manager()
      {
        using namespace we::mgmt::detail::commands;
        std::cerr << "D: manager thread started..." << std::endl;
        barrier_.wait();
        for (;;)
        {
          cmd_t cmd = cmd_q_.get();
          try
          {
            cmd.handle();
          }
          catch (std::exception const& ex)
          {
            std::cerr << "W: error during command handling: " << ex.what() << std::endl;
          }
        }
        std::cerr << "D: manager thread stopped..." << std::endl;
      }

      inline
      bool is_alive ( const activity_type & act ) const
      {
        return act.is_alive();
      }

      inline
      bool is_done ( const activity_type & act ) const
      {
        return ! (act.has_enabled() || activity_child_count (act.id()));
      }

      inline
      size_t activity_child_count ( const internal_id_type & id ) const
      {
        typename parent_to_children_map_t::const_iterator children (parent_to_child_.find(id));
        if (children != parent_to_child_.end())
        {
          return children->second.size();
        }
        else
        {
          return 0;
        }
      }

      inline
      void post_execute_externally ( const internal_id_type & id)
      {
        // create external id
        external_id_type ext_id ( external_id_gen_() );
        add_external_to_internal_mapping ( ext_id, id );
        ext_submit ( ext_id, policy::codec::encode ( lookup (id) ) );
      }

      inline
      void post_activity_notification( const internal_id_type & id)
      {
        cmd_q_.put(make_cmd(id, boost::bind(&this_type::activity_needs_attention, this, _1)));
      }

      inline
      void post_finished_notification( const internal_id_type & id)
      {
        cmd_q_.put(make_cmd(id, boost::bind(&this_type::activity_finished, this, _1)));
      }

      inline
      void post_failed_notification( const internal_id_type & id)
      {
        cmd_q_.put(make_cmd(id, boost::bind(&this_type::activity_failed, this, _1)));
      }

      inline
      void post_cancelled_notification( const internal_id_type & id)
      {
        cmd_q_.put(make_cmd(id, boost::bind(&this_type::activity_cancelled, this, _1)));
      }

      inline
      void post_suspend_activity_notification( const internal_id_type & id )
      {
        cmd_q_.put (make_cmd(id, boost::bind(&this_type::suspend_activity, this, _1)));
      }

      inline
      void post_resume_activity_notification( const internal_id_type & id )
      {
        cmd_q_.put (make_cmd(id, boost::bind(&this_type::resume_activity, this, _1)));
      }

      inline
      void post_execute_notification ( const internal_id_type & id )
      {
        exec_q_.put ( id );
      }

      inline
      void post_inject_activity_results ( const internal_id_type & id )
      {
        inj_q_.put ( id );
      }

      void extractor()
      {
        using namespace we::mgmt::detail::commands;
        std::cerr << "D: extractor thread started..." << std::endl;
        barrier_.wait();
        for (;;)
        {
          // TODO: probably timed wait?
          internal_id_type active_id = active_nets_.get();

          try
          {
            activity_type & act = lookup(active_id);

            if (is_done (act))
            {
              std::cerr << "D: act[" << active_id << "] is done." << std::endl;
              post_finished_notification (active_id);
              continue;
            }

            // TODO: check status flags
            if (! is_alive (act))
            {
              std::cerr << "D: act[" << active_id << "] is on hold." << std::endl;
              continue;
            }

            // submit new activities
            while (act.has_enabled())
            {
              std::cerr << "I: act["
                        << active_id
                        << "] has enabled transition(s)"
                        << std::endl;

              activity_type sub_act = act.extract();

              const internal_id_type sub_act_id = internal_id_gen_();
              insert_activity (sub_act_id, sub_act);

              // TODO:
              // establish parent <-> child relationship
              establish_parent_child_relationship ( active_id, sub_act_id );
              post_execute_notification (sub_act_id);
            }
          }
          catch (const activity_not_found<id_type> & ex)
          {
            std::cerr << "W: activity could not be found: " << ex.id << std::endl;
          }
        }
        std::cerr << "D: extractor thread stopped..." << std::endl;
      }

      inline
      void establish_parent_child_relationship ( const internal_id_type & parent
                                               , const internal_id_type & child
                                               )
      {
        parent_to_child_[parent].insert (child);
        child_to_parent_[child] = parent;
      }

      inline
      void remove_parent_child_relationship ( const internal_id_type & parent
                                            , const internal_id_type & child
                                            )
      {
        child_to_parent_.erase (child);
        parent_to_child_[parent].erase (child);
      }

      inline
      internal_id_type parent_of ( const internal_id_type & id ) const
      {
        if (has_parent (id))
        {
          return child_to_parent_.at (id);
        }
        else
        {
          throw exception::no_such_mapping<internal_id_type> ( "child id := "
                                                             + ::util::show(id)
                                                             + " does not have a parent!"
                                                             , id
                                                             );
        }
      }

      inline
      bool has_parent (const internal_id_type & id) const
      {
        typename child_to_parent_map_t::const_iterator p (child_to_parent_.find( id ));
        if (p == child_to_parent_.end())
        {
          return false;
        }
        else
        {
          return true;
        }
      }

      void injector()
      {
        std::cerr << "D: injector thread started..." << std::endl;
        barrier_.wait();
        for (;;)
        {
          inj_cmd_t cmd = inj_q_.get();
          try
          {
            const internal_id_type act_id = cmd;
            const internal_id_type par_id = parent_of (cmd);

            activity_type & act = lookup( act_id );
            activity_type & par = lookup( par_id );
            std::cerr << "I: injecting results of "
                      << "act[" << act_id << "] into "
                      << "act[" << par_id << "]"
                      << std::endl;

            act.collect_output ();
            par.inject (act);
            remove_activity ( act_id );

            post_activity_notification( par_id );
          }
          catch (const std::exception & ex)
          {
            std::cerr << "E: error during injecting: " << ex.what() << std::endl;
            // ignore
          }
        }
        std::cerr << "D: injector thread stopped..." << std::endl;
      }

      void executor()
      {
        std::cerr << "D: executor thread started..." << std::endl;
        barrier_.wait();
        for (;;)
        {
          executor_cmd_t cmd = exec_q_.get();
          try
          {
            const internal_id_type act_id = cmd;

            std::cerr << "I: executing id := " << act_id << std::endl;
            activity_type & act = lookup (act_id);

            try
            {
              static typename policy::exec_policy exec_policy
                ( boost::bind ( &this_type::post_activity_notification, this, _1 )
                , boost::bind ( &this_type::post_execute_externally, this, _1 )
                , boost::bind ( &this_type::post_inject_activity_results, this, _1 )
                );

              sig_executing (this, act_id, policy::codec::encode (act));

              act.inject_input ();
              act.execute (exec_policy);
            }
            catch (std::exception const & ex)
            {
              // activity failed
            }
          } catch (const std::exception & ex)
          {
            std::cerr << "E: error during execution: " << ex.what() << std::endl;
            // ignore
          }
        }
        std::cerr << "D: executor thread stopped..." << std::endl;
      }

      void print_activity_info (std::ostream & s, const activity_type & act) const
      {
        we::mgmt::type::detail::printer <activity_type> p (act, s);
        p << "   **** activity [" << act.id() << "]:" << std::endl;
        p << "         name := " << act.transition().name() << std::endl;
        p << "     internal := " << act.transition().is_internal() << std::endl;
        p << "         type := " << act.type_to_string () << std::endl;
        p << "        input := " << act.input() << std::endl;
        p << "       output := " << act.output() << std::endl;
        p << "       parent := " << (has_parent (act.id()) ? ::util::show(parent_of (act.id())) : "n/a") << std::endl;
        p << "     children := [";
        typename parent_to_children_map_t::const_iterator children (parent_to_child_.find(act.id()));
        if (children != parent_to_child_.end())
        {
          for ( typename child_set_t::const_iterator child (children->second.begin())
              ; child != children->second.end()
              ; ++child
              )
          {
            if (child != children->second.begin())
            {
              p << ", ";
            }
            p << *child;
          }
        }
        p << "]";
        p << std::endl;
      }

      /** Member variables **/
    private:
      boost::function<external_id_type()> external_id_gen_;
      boost::function<internal_id_type()> internal_id_gen_;
      boost::barrier barrier_;
      mutable boost::shared_mutex activities_mutex_;
      mutable boost::shared_mutex id_map_mutex_;
      activities_t activities_;
      cmd_q_t cmd_q_;
      active_nets_t active_nets_;
      inj_q_t inj_q_;
      exec_q_t exec_q_;

      external_to_internal_map_t ex_to_in_;
      internal_to_external_map_t in_to_ex_;

      parent_to_children_map_t parent_to_child_;
      child_to_parent_map_t child_to_parent_;

      boost::mutex active_nets_mutex_;
      boost::condition active_nets_modified_;

      boost::thread extractor_;
      boost::thread manager_;
      boost::thread injector_;
      boost::thread executor_;

      void activity_needs_attention(const cmd_t & cmd)
      {
        activity_type & a = lookup ( cmd.dat );
        if (! a.flags().suspended )
        {
          std::cerr << "D: act[" << cmd.dat << "] active" << std::endl;
          active_nets_.put (cmd.dat);
        }
      }

      void activity_finished(const cmd_t & cmd)
      {
        const internal_id_type internal_id (cmd.dat);

        std::cerr << "D: act[" << internal_id << "] finished" << std::endl;

        lookup (internal_id).collect_output ();
        sig_finished (this, internal_id, policy::codec::encode(lookup(internal_id)));

        if ( has_parent (internal_id) )
        {
          post_inject_activity_results (internal_id);
        }
        else
        {
          ext_finished (map_to_external (internal_id), policy::codec::encode (lookup (internal_id)));

          assert_is_leaf ( cmd.dat );
          remove_activity ( cmd.dat );
        }
      }

      void activity_failed(const cmd_t & cmd)
      {
        const internal_id_type internal_id (cmd.dat);

        std::cerr << "D: act[" << internal_id << "] failed" << std::endl;

        lookup (internal_id).collect_output ();
        sig_finished (this, internal_id, policy::codec::encode(lookup(internal_id)));

        if ( has_parent (internal_id) )
        {
          // TODO cancel strategy
          post_inject_activity_results (internal_id);
        }
        else
        {
          ext_failed (map_to_external (internal_id), policy::codec::encode (lookup (internal_id)));

          assert_is_leaf ( cmd.dat );
          remove_activity ( cmd.dat );
        }
      }

      void activity_cancelled(const cmd_t & cmd)
      {
        sig_cancelled (this, cmd.dat, policy::codec::encode(lookup(cmd.dat)) );

        assert_is_leaf ( cmd.dat );
        remove_activity ( cmd.dat );
        std::cerr << "D: act[" << cmd.dat << "] cancelled" << std::endl;
      }

      inline
      void assert_is_leaf (const internal_id_type &id) const
      {
        if (activity_child_count (id) > 0)
        {
          throw std::runtime_error("not leaf: activity-id := " + ::util::show (id));
        }
      }

      void suspend_activity(const cmd_t & cmd)
      {
        //        lookup(cmd.dat).flags().suspended = true;
        std::cerr << "I: act[" << cmd.dat << "] suspended" << std::endl;
      }

      void resume_activity(const cmd_t & cmd)
      {
        //        lookup(cmd.dat).flags().suspended = false;
        active_nets_.put (cmd.dat);
        std::cerr << "I: act[" << cmd.dat << "] resumed" << std::endl;
      }

      void async_execute(activity_type & act)
      {
        std::cerr << "D: async_execute " << act.id() << std::endl;
        post_execute_notification (act.id());
      }

      inline void insert_activity(const internal_id_type & id, activity_type & act)
      {
        boost::unique_lock<boost::shared_mutex> lock (activities_mutex_);
        act.set_id (id);
        activities_.insert(std::make_pair(id, act));
      }

      inline void remove_activity(const activity_type & act)
      {
        remove_activity (act.id());
      }

      inline void remove_activity(const internal_id_type & id)
      {
        boost::unique_lock<boost::shared_mutex> lock (activities_mutex_);
        try
        {
          internal_id_type parent ( parent_of (id) );
          remove_parent_child_relationship ( parent, id );
        }
        catch (const exception::no_such_mapping<internal_id_type> & )
        {
          // ignore, don't have a parent
        }
        activities_.erase (id);
      }

      inline activity_type & lookup(const internal_id_type & id)
      {
        boost::shared_lock <boost::shared_mutex> lock (activities_mutex_);
        typename activities_t::iterator a = activities_.find(id);
        if (a == activities_.end()) throw activity_not_found<internal_id_type>("lookup("+::util::show(id)+") failed!", id);
        return a->second;
      }

      inline const activity_type & lookup(const internal_id_type & id) const
      {
        boost::shared_lock <boost::shared_mutex> lock (activities_mutex_);
        typename activities_t::const_iterator a = activities_.find(id);
        if (a == activities_.end()) throw activity_not_found<internal_id_type>("lookup("+::util::show(id)+") failed!", id);
        return a->second;
      }

      friend class boost::serialization::access;
      template<class Archive>
      void serialize (Archive & ar, const unsigned int)
      {
        //        ar & activities_;
        //        ar & active_nets_;
        // TODO: serialize queues
        //        ar & ex_to_in_;
        //        ar & in_to_ex_;
      }
    };
  }}

#endif
