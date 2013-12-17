// mirko.rahn@itwm.fraunhofer.de

#include <we/mgmt/layer.hpp>

namespace we
{
  namespace mgmt
  {
    void layer::submit ( const external_id_type& id
                       , const encoded_type& bytes
                       , const we::type::user_data & data
                       )
    {
      submit (id, we::mgmt::type::activity_t (bytes), data);
    }
    void layer::submit ( const external_id_type& id
                       , const we::mgmt::type::activity_t& act
                       , const we::type::user_data & data
                       )
    {
      detail::descriptor desc (generate_internal_id(), act);
      desc.set_user_data (data);
      desc.came_from_external_as (id);
      desc.inject_input ();

      submit (desc);
    }
    void layer::submit (detail::descriptor const &desc)
    {
      lock_t const _ (mutex_);

      insert_activity(desc);
      active_nets_.put (desc.id ());
    }

    void layer::cancel (const external_id_type& id, const reason_type& reason)
    {
      lock_t const _ (mutex_);

      internal_id_type const int_id (map_to_internal (id));

      const std::string message
        (reason.empty () ? "user requested cancellation" : reason.c_str ());

      MLOG (WARN, "cancel ( " << id << " ) := " << message);

      {
        lookup (int_id).set_error_code (ECANCELED).set_error_message (message);
        cancel_activity (int_id);
      }
    }
    void layer::finished (const external_id_type& id, const result_type& result)
    {
      lock_t const _ (mutex_);

      internal_id_type const int_id (map_to_internal (id));

      {
        lookup (int_id).output (we::mgmt::type::activity_t (result).output());
        do_inject (lookup (int_id));
      }
    }
    void layer::failed ( const external_id_type& id
                       , const result_type& result
                       , const int error_code
                       , const std::string& error_message
                       )
    {
      lock_t const _ (mutex_);

      detail::descriptor& d (lookup (map_to_internal (id)));

      d.set_error_code (error_code);
      d.set_error_message (d.name () + ": " + error_message);
      d.set_result (result);

      // TODO:
      //    lookup activity
      //    mark as failed
      //    store result + reason

      //    let the "parent" fail as well
      //        -> mark the parent's *outcome* as "failed"
      //        -> store the reasons, i.e. a list of childfailures
      //        -> cancel all children
      //        -> wait until all children are done
      //        -> inform parent and so on
      activity_failed (d.id ());
    }
    void layer::canceled (const external_id_type& id)
    {
      lock_t const _ (mutex_);

      DMLOG (WARN, "canceled(" << id << ")");
      activity_canceled (map_to_internal (id));
    }

    void layer::add_map_to_internal ( const external_id_type & external_id
                                    , const internal_id_type & internal_id
                                    )
    {
      lock_t const _ (mutex_);

      ext_to_int_.insert ( external_to_internal_map_t::value_type
                         (external_id, internal_id)
                         );
    }
    void layer::del_map_to_internal ( const external_id_type & external_id
                                    , const internal_id_type & internal_id
                                    )
    {
      lock_t const _ (mutex_);
      ext_to_int_.erase (external_id);
    }

    void layer::del_map_to_internal (const internal_id_type & internal_id)
    {
      lock_t const _ (mutex_);
      external_to_internal_map_t::iterator it = ext_to_int_.begin ();
      const external_to_internal_map_t::iterator end = ext_to_int_.end ();

      while (it != end)
      {
        if (it->second == internal_id)
        {
          it = ext_to_int_.erase (it);
        }
        else
        {
          ++it;
        }
      }
    }

    layer::external_to_internal_map_t::mapped_type layer::map_to_internal ( const external_id_type & external_id ) const
    {
      lock_t const _ (mutex_);

      return ext_to_int_.at (external_id);
    }

    void layer::execute_externally (const internal_id_type & int_id)
    {
      detail::descriptor& desc (lookup (int_id));

      external_id_type ext_id ( generate_external_id() );
      desc.sent_to_external_as (ext_id);
      add_map_to_internal (ext_id, int_id);

      we::mgmt::type::activity_t ext_act (desc.activity());
      ext_act.transition().set_internal(true);

      DMLOG( TRACE
           , "submitting internal activity " << int_id
           << " to external with id " << ext_id
           << " related to " << desc.get_user_data ().get_user_job_identification ()
           );

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
    void layer::executor ()
    {
      for (;;)
      {
        internal_id_type const active_id (active_nets_.get());
        if (! is_valid (active_id))
        {
          DMLOG( WARN
               , "executor woken up by old activity id " << active_id
               );
          continue;
        }

        detail::descriptor& desc (lookup(active_id));

        if (desc.activity().is_canceling())
        {
          if (!desc.has_children())
          {
            activity_canceled (active_id);
          }

          continue;
        }

        //! \todo: check status flags
        if (! desc.is_alive ())
        {
          DMLOG (DEBUG, "activity (" << desc.name() << ")-" << active_id << " is on hold");
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
          MLOG( ERROR
              , "executor: something went wrong during execution of: "
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

          activity_failed (desc.id ());
        }
      }
      DMLOG(TRACE, "executor thread stopped...");
    }

    detail::descriptor &layer::do_extract (detail::descriptor & parent)
    {
      internal_id_type id (generate_internal_id ());
      insert_activity (parent.extract(id));
      return lookup (id);
    }

    void layer::do_execute (detail::descriptor& desc)
    {
      lock_t const _ (mutex_);

      if (desc.is_done ())
      {
        DMLOG (TRACE, "executor: activity (" << desc.name() << ")-" << desc.id() << " is done");
        do_inject (desc);
      }
      else
      {
        policy::execution_policy exec_policy;

        switch (desc.execute (&exec_policy))
        {
        case policy::execution_policy::EXTRACT:
          {
            while (desc.is_alive () && desc.enabled())
            {
              detail::descriptor & child (do_extract (desc));
              child.inject_input ();

              switch (child.execute (&exec_policy))
              {
              case policy::execution_policy::EXTRACT:
                active_nets_.put (child.id ());
                break;
              case policy::execution_policy::INJECT:
                do_inject (child);
                break;
              case policy::execution_policy::EXTERNAL:
                execute_externally (child.id());
                break;
              default:
                throw std::runtime_error ("invalid classification during execution of activity: " + fhg::util::show (child));
              }
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
          MLOG (FATAL, "executor: got strange classification for activity (" << desc.name() << ")-" << desc.id());
          throw std::runtime_error ("executor got strange classification for activity");
        }
      }
    }
    bool layer::is_valid (const internal_id_type & id) const
    {
      lock_t const _ (mutex_);
      return activities_.find(id) != activities_.end();
    }
    void layer::do_inject (detail::descriptor& desc)
    {
      desc.finished();

      if (desc.has_parent())
      {
        lookup (desc.parent()).inject (desc);
        active_nets_.put (desc.parent ());
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

    void layer::activity_failed (internal_id_type const internal_id)
    {
      detail::descriptor& desc (lookup(internal_id));
      desc.failed();

      DMLOG ( WARN
            , "failed (" << desc.name() << ")-" << desc.id() << " : "
            << desc.error_message ()
            );

      if (desc.has_children())
      {
        DMLOG (WARN, "cancelling all children of" << std::endl << desc);

        desc.cancel
          (boost::bind ( &layer::cancel_activity
                       , this
                       , _1
                       )
          );
      }
      else if (desc.has_parent ())
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
        if (not parent_desc.activity ().is_canceling () && not parent_desc.activity ().is_failed ())
        {
          cancel_activity (parent_desc.id ());
        }

        remove_activity (desc);
      }
      else if (desc.came_from_external ())
      {
        ext_failed ( desc.from_external_id()
                   , desc.activity().to_string()
                   , desc.error_code()
                   , desc.error_message()
                   );
        remove_activity (desc);
      }
      else
      {
        throw std::runtime_error ("activity failed, but I don't know what to do with it: " + fhg::util::show (desc));
      }
    }

    void layer::activity_canceled (internal_id_type const internal_id)
    {
      detail::descriptor& desc (lookup(internal_id));
      if (not desc.activity ().is_failed ())
      {
        desc.canceled ();
      }

      if (desc.has_parent ())
      {
        DLOG ( INFO, "activity "
             << desc.name ()
             << " canceled and has a parent: reason := "
             << desc.error_message ()
             );

        detail::descriptor& parent (lookup (desc.parent()));
        parent.child_canceled (desc, desc.error_code (), desc.error_message ());

        if (! parent.has_children () && not parent.activity ().is_failed ())
        {
          activity_canceled (parent.id ());
        }
        else
        {
          active_nets_.put (parent.id ());
        }
      }
      else if (desc.came_from_external ())
      {
        // if we were cancelling because of a failed child, notify failed
        if (desc.activity ().is_failed ())
        {
          DMLOG (WARN, "descriptor failed: " << std::endl << desc);

          ext_failed ( desc.from_external_id()
                     , desc.activity().to_string()
                     , desc.error_code()
                     , desc.error_message()
                     );
        }
        else
        {
          DMLOG (WARN, "descriptor canceled: " << std::endl << desc);

          ext_canceled (desc.from_external_id());
        }
      }
      else
      {
        throw std::runtime_error ("activity canceled, but I don't know what to do with it: " + fhg::util::show (desc));
      }

      remove_activity (desc);
    }

    void layer::cancel_activity (internal_id_type const internal_id)
    {
      lock_t const _ (mutex_);

      DMLOG (WARN, "cancel_activity(" << internal_id << ")");

      detail::descriptor& desc (lookup(internal_id));

      if (desc.activity ().is_canceling ())
        return;

      if (desc.has_children ())
      {
        desc.cancel
          ( boost::bind ( &layer::cancel_activity
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
                     , desc.error_message ()
                     );
        }
      }
      else
      {
        activity_canceled (desc.id ());
      }
    }

    void layer::insert_activity(const detail::descriptor & desc)
    {
      lock_t const _ (mutex_);

      activities_.insert(std::make_pair(desc.id(), desc));
      if (desc.came_from_external())
      {
        add_map_to_internal (desc.from_external_id(), desc.id());
      }

      if (sig_insert)
      {
        (*sig_insert) (desc);
      }
    }

    void layer::remove_activity(const detail::descriptor & desc)
    {
      lock_t const _ (mutex_);

      if (sig_remove)
      {
        (*sig_remove) (desc);
      }

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

      active_nets_.remove_if
        (boost::bind (&std::equal_to<internal_id_type>::operator(), std::equal_to<internal_id_type>(), desc.id (), _1));
      activities_.erase (desc.id());
    }

    detail::descriptor& layer::lookup (const internal_id_type& id)
    {
      lock_t const _ (mutex_);

      try
      {
        return activities_.at (id);
      }
      catch (std::exception const&)
      {
        throw std::invalid_argument
          ((boost::format ("unable to locate id: %1%") % id).str ());
      }
    }
  }
}
