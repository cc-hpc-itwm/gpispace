// alexander.petry@itwm.fraunhofer.de

#ifndef WE_MGMT_LAYER_HPP
#define WE_MGMT_LAYER_HPP 1

#include <we/mgmt/bits/descriptor.hpp>
#include <we/mgmt/bits/execution_policy.hpp>
#include <we/mgmt/bits/signal.hpp>
#include <we/mgmt/exception.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/type/id.hpp>
#include <we/type/net.hpp>
#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>

#include <fhg/assert.hpp>
#include <fhg/error_codes.hpp>
#include <fhg/util/show.hpp>
#include <fhg/util/thread/queue.hpp>
#include <fhg/util/threadname.hpp>

#include <fhglog/fhglog.hpp>

#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

namespace we
{
  namespace mgmt
  {
    class layer
    {
    public:
      typedef std::string id_type;

      typedef std::string external_id_type;
      typedef petri_net::activity_id_type internal_id_type;

      typedef std::string encoded_type;
      typedef std::string reason_type;
      typedef std::string result_type;

    private:
      typedef boost::unordered_map<external_id_type, internal_id_type> external_to_internal_map_t;

      typedef boost::unordered_map< internal_id_type
                                  , detail::descriptor
                                  > activities_t;

      typedef fhg::thread::queue<boost::function<void ()> > cmd_q_t;

      //! \todo is it necessary to use a locked data structure?
      typedef fhg::thread::queue<internal_id_type> active_nets_t;

      typedef boost::unique_lock<boost::recursive_mutex> lock_t;

    public:

      /******************************
       * EXTERNAL API
       *****************************/

      // observe
      util::signal<void (detail::descriptor)> sig_insert;
      util::signal<void (detail::descriptor)> sig_remove;

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
      void cancel (const external_id_type&, const reason_type&);

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
      void finished (const external_id_type&, const result_type&);

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
      void failed ( const external_id_type&
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
      void canceled (const external_id_type&);

      // END: EXTERNAL API

    private:
      // handle execution layer
      boost::function<void ( external_id_type const &
                           , encoded_type const &
                           , const std::list<we::type::requirement_t>&
                           , const we::type::schedule_data&
                           , const we::type::user_data &
                           )> ext_submit;
      boost::function<void ( external_id_type const &
                           , reason_type const &
                           )>  ext_cancel;
      boost::function<void ( external_id_type const &
                           , result_type const &
                           )>  ext_finished;
      boost::function<void ( external_id_type const &
                           , result_type const &
                           , const int error_code
                           , std::string const & reason
                           )>  ext_failed;
      boost::function<void (external_id_type const &)> ext_canceled;

      void submit (const detail::descriptor & desc)
      {
        insert_activity(desc);

        post_execute_notification (desc.id());
      }

      void add_map_to_internal ( const external_id_type & external_id
                               , const internal_id_type & internal_id
                               )
      {
        lock_t const _ (mutex_);

        ext_to_int_.insert ( external_to_internal_map_t::value_type
                           (external_id, internal_id)
                           );
      }

      void del_map_to_internal ( const external_id_type & external_id
                               , const internal_id_type & internal_id
                               )
      {
        lock_t const _ (mutex_);

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
      }

      external_to_internal_map_t::mapped_type map_to_internal ( const external_id_type & external_id ) const
      {
        lock_t const _ (mutex_);

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
      template <class E>
        layer (E* exec_layer, boost::function<external_id_type()> gen)
          : sig_insert()
          , sig_remove()
          , ext_submit (boost::bind (&E::submit, exec_layer, _1, _2, _3, _4, _5))
          , ext_cancel (boost::bind (&E::cancel, exec_layer, _1, _2))
          , ext_finished (boost::bind (&E::finished, exec_layer, _1, _2))
          , ext_failed (boost::bind (&E::failed, exec_layer, _1, _2, _3, _4))
          , ext_canceled (boost::bind (&E::canceled, exec_layer, _1))
          , external_id_gen_ (gen)
          , internal_id_gen_ (&petri_net::activity_id_generate)
          , manager_ (boost::bind (&layer::manager, this))
          , extractor_ (boost::bind (&layer::extractor, this))
          , injector_ (boost::bind (&layer::injector, this))
      {
        fhg::util::set_threadname (manager_, "[we-mgr]");
        fhg::util::set_threadname (extractor_, "[we-extract]");
        fhg::util::set_threadname (injector_, "[we-inject]");
      }

      ~layer()
      {
        manager_.interrupt();
        if (manager_.joinable())
        {
          manager_.join();
        }

        injector_.interrupt();
        if (injector_.joinable())
        {
          injector_.join();
        }

        extractor_.interrupt();
        if (extractor_.joinable())
        {
          extractor_.join();
        }
      }

    private:
      void manager()
      {
        for (;;)
        {
          cmd_q_.get()();
        }
      }

      inline
        void post_activity_notification (const internal_id_type & id)
      {
        if (is_valid(id))
        {
          active_nets_.put(id);
        }
        else
        {
          throw std::runtime_error
            ("post_activity_notification: id is not valid anymore");
        }
      }

      inline
        void post_finished_notification (const internal_id_type & id)
      {
        if (is_valid(id))
        {
          inj_q_.put (id);
        }
        else
        {
          throw std::runtime_error
            ("post_finished_notification: id is not valid anymore");
        }
      }

      inline
        void post_failed_notification (const internal_id_type & id)
      {
        if (is_valid(id))
        {
          cmd_q_.put (boost::bind (&layer::activity_failed, this, id));
        }
        else
        {
          throw std::runtime_error
            ("post_failed_notification: id is not valid anymore");
        }
      }

      inline
        void post_canceled_notification (const internal_id_type & id)
      {
        if (is_valid(id))
        {
          cmd_q_.put (boost::bind (&layer::activity_canceled, this, id));
        }
        else
        {
          throw std::runtime_error
            ("post_canceled_notification: id is not valid anymore");
        }
      }

      inline
        void post_cancel_activity_notification (const internal_id_type & id)
      {
        if (is_valid(id))
        {
          cmd_q_.put (boost::bind (&layer::cancel_activity, this, id));
        }
        else
        {
          throw std::runtime_error
            ("post_cancel_activity_notification: id is not valid anymore");
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
          throw std::runtime_error
            ("post_execute_notification: id is not valid anymore");
        }
      }

      inline
        void execute_externally (const internal_id_type & int_id)
      {
        detail::descriptor& desc (lookup (int_id));

        external_id_type ext_id ( generate_external_id() );
        desc.sent_to_external_as (ext_id);
        add_map_to_internal (ext_id, int_id);

        we::mgmt::type::activity_t ext_act (desc.activity());
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
                   , desc.get_user_data ()
                   );
      }

      //! \todo WORK HERE: rewrite!
      void extractor()
      {
        for (;;)
        {
          internal_id_type const active_id (active_nets_.get());
          if (! is_valid (active_id))
          {
            DLOG( WARN
                , "extractor woken up by old activity id " << active_id
                );
            continue;
          }

            detail::descriptor& desc (lookup(active_id));

            if (desc.activity().is_canceling())
            {
              if (!desc.has_children())
              {
                post_canceled_notification (active_id);
              }

              continue;
            }

            //! \todo: check status flags
            if (! desc.is_alive ())
            {
              LOG(DEBUG, "activity (" << desc.name() << ")-" << active_id << " is on hold");
              continue;
            }

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
                 << desc.name() << ": " << ex.what()
                 );
              desc.set_error_code (fhg::error::UNEXPECTED_ERROR);
              desc.set_error_message
                ( std::string ("in: '")
                + desc.name ()
                + std::string ("' ")
                + ex.what ()
                );
              desc.set_result (desc.activity().to_string());
              post_failed_notification (desc.id());
            }
        }
        DLOG(INFO, "extractor thread stopped...");
      }

      inline
        void do_execute (detail::descriptor& desc)
      {
        policy::execution_policy exec_policy;

        switch (desc.execute (&exec_policy))
        {
        case policy::execution_policy::EXTRACT:
          {
            while (desc.is_alive () && desc.enabled())
            {
              detail::descriptor child (desc.extract(generate_internal_id()));
              child.inject_input ();
              child.set_user_data (desc.get_user_data ());

              switch (child.execute (&exec_policy))
              {
              case policy::execution_policy::EXTRACT:
                insert_activity(child);
                post_execute_notification (child.id());
                break;
              case policy::execution_policy::INJECT:
                child.finished();
                desc.inject (child);
                break;
              case policy::execution_policy::EXTERNAL:
                insert_activity (child);
                execute_externally (child.id());
                break;
              default:
                throw std::runtime_error ("invalid classification during execution of activity: " + fhg::util::show (child));
              }
            }

            if (desc.is_done ())
            {
              DLOG(DEBUG, "extractor: activity (" << desc.name() << ")-" << desc.id() << " is done");
              do_inject (desc);
            }
          }
          break;
        case policy::execution_policy::INJECT:
          do_inject (desc);
          break;
        case policy::execution_policy::EXTERNAL:
          execute_externally (desc.id());
          break;
        default:
          LOG(FATAL, "extractor: got strange classification for activity (" << desc.name() << ")-" << desc.id());
          throw std::runtime_error ("extractor got strange classification for activity");
        }
      }

      inline
        bool is_valid (const internal_id_type & id) const
      {
        lock_t const _ (mutex_);
        return activities_.find(id) != activities_.end();
      }

      void injector()
      {
        for (;;)
        {
          const internal_id_type act_id = inj_q_.get();

          if ( ! is_valid (act_id))
          {
            DLOG(WARN, "injector woken up by old activity id " << act_id);
            continue;
          }

          detail::descriptor& desc (lookup (act_id));

          try
          {
            do_inject (desc);
          }
          catch (std::exception const & ex)
          {
            LOG(ERROR, "injector got exception during injecting: " << ex.what());

            desc.set_error_code (fhg::error::UNEXPECTED_ERROR);
            desc.set_error_message (ex.what ());
            desc.set_result (desc.activity ().to_string());

            post_failed_notification (desc.id ());
          }
        }
        DLOG(INFO, "injector thread stopped...");
      }

      void do_inject (detail::descriptor& desc)
      {
        desc.finished();

        if (desc.has_parent())
        {
          lookup (desc.parent()).inject
            ( desc
            , boost::bind ( &layer::post_activity_notification
                          , this
                          , _1
                          )
            );
        }
        else if (desc.came_from_external())
        {
          if (desc.activity ().is_failed ())
          {
            DLOG ( INFO
                 , "failed (" << desc.name() << ")-" << desc.id()
                 << " external-id := " << desc.from_external_id()
                 );
            ext_failed ( desc.from_external_id()
                       , desc.activity().to_string()
                       , desc.error_code()
                       , desc.error_message()
                       );
          }
          else if (desc.activity ().is_canceled ())
          {
            DLOG ( INFO
                 , "canceled (" << desc.name() << ")-" << desc.id()
                 << " external-id := " << desc.from_external_id()
                 );
            ext_canceled (desc.from_external_id());
          }
          else
          {
            DLOG ( INFO
                 , "finished (" << desc.name() << ")-" << desc.id()
                 << " external-id := " << desc.from_external_id()
                 );
            ext_finished ( desc.from_external_id()
                         , desc.activity().to_string()
                         );
          }
        }
        else
        {
          throw std::runtime_error ("STRANGE! cannot inject: " + fhg::util::show (desc));
        }

        remove_activity (desc);
      }

    private:
      boost::function<external_id_type()> external_id_gen_;
      boost::function<internal_id_type()> internal_id_gen_;
      mutable boost::recursive_mutex mutex_;
      mutable boost::recursive_mutex id_gen_mutex_;
      activities_t activities_;

      cmd_q_t cmd_q_;
      active_nets_t active_nets_;
      active_nets_t inj_q_;

      external_to_internal_map_t ext_to_int_;

      boost::thread manager_;
      boost::thread extractor_;
      boost::thread injector_;

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

      void activity_failed (internal_id_type const internal_id)
      {
          detail::descriptor& desc (lookup(internal_id));
          desc.failed();

          DMLOG ( WARN
                , "failed (" << desc.name() << ")-" << desc.id() << " : "
                << desc.error_message ()
                );

          if (desc.has_parent ())
          {
            detail::descriptor& parent_desc (lookup (desc.parent()));

            parent_desc.child_failed ( desc
                                     , desc.error_code()
                                     , desc.error_message()
                                     );
            //! \fixme: handle failure in a meaningful way:
            //     - check failure reason
            //         - EAGAIN reschedule (had not been submitted yet)
            //         - ECRASH activity crashed (idempotence criteria)
            post_cancel_activity_notification (parent_desc.id ());
          }
          else if (desc.came_from_external ())
          {
            ext_failed ( desc.from_external_id()
                       , desc.activity().to_string()
                       , desc.error_code()
                       , desc.error_message()
                       );
          }
          else
          {
            throw std::runtime_error ("activity failed, but I don't know what to do with it: " + fhg::util::show (desc));
          }

          remove_activity (desc);
      }

      void activity_canceled (internal_id_type const internal_id)
      {
          detail::descriptor& desc (lookup(internal_id));
          desc.canceled();

          if (desc.has_parent ())
          {
            DLOG ( INFO, "activity "
                 << desc.name ()
                 << " canceled and has a parent: reason := "
                 << desc.error_message ()
                 );

            detail::descriptor& parent (lookup (desc.parent()));
            parent.child_canceled(desc, "TODO: child canceled reason");

            if (! parent.has_children ())
            {
              post_canceled_notification (parent.id());
            }
            else
            {
              post_cancel_activity_notification (parent.id ());
            }
          }
          else if (desc.came_from_external ())
          {
            ext_failed ( desc.from_external_id()
                       , desc.activity().to_string()
                       , desc.error_code()
                       , desc.error_message()
                       );
          }
          else
          {
            throw std::runtime_error ("activity canceled, but I don't know what to do with it: " + fhg::util::show (desc));
          }

          remove_activity (desc);
      }

      void cancel_activity (internal_id_type const internal_id)
      {
          detail::descriptor& desc (lookup(internal_id));

          if (desc.has_children())
          {
            desc.cancel
              ( boost::bind ( &layer::post_cancel_activity_notification
                            , this
                            , _1
                            )
              );
          }
          else if (desc.sent_to_external())
          {
            if (! (  desc.activity().is_canceling()
                  || desc.activity().is_failed()
                  || desc.activity().is_canceled()
                  || desc.activity().is_finished()
                  )
               )
            {
              ext_cancel ( desc.to_external_id()
                         , "WFE policy cancel-on-failure in place"
                         );
            }
          }
          else
          {
            post_canceled_notification (desc.id());
          }
      }

      inline void insert_activity(const detail::descriptor & desc)
      {
        lock_t const _ (mutex_);

        activities_.insert(std::make_pair(desc.id(), desc));
        if (desc.came_from_external())
        {
          add_map_to_internal (desc.from_external_id(), desc.id());
        }

        sig_insert (desc);
      }

      inline void remove_activity(const detail::descriptor & desc)
      {
        lock_t const _ (mutex_);

        sig_remove (desc);

        if (desc.has_children())
          throw std::runtime_error("cannot remove non-leaf: " + fhg::util::show (desc));
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

      inline detail::descriptor& lookup (const internal_id_type& id)
      {
        lock_t const _ (mutex_);

        return activities_.at (id);
      }
    };
  }
}

#endif
