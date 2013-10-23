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
      DLOG (TRACE, "submit (" << id << ", ...)");

      return submit (id, we::mgmt::type::activity_t (bytes), data);
    }
    void layer::submit ( const external_id_type& id
                       , const we::mgmt::type::activity_t& act
                       , const we::type::user_data & data
                       )
    {
      descriptor_ptr desc
        (new detail::descriptor (generate_internal_id(), act));
      desc->set_user_data (data);
      desc->came_from_external_as (id);
      desc->inject_input ();

      submit (desc);
    }
    bool layer::cancel (const external_id_type& id, const reason_type& reason)
    {
      fhg::util::remove_unused_variable_warning (reason);

      MLOG (WARN, "cancel ( " << id << " ) := " << reason);

      try
        {
          post_cancel_activity_notification (map_to_internal (id));
        }
      catch (std::exception const &)
        {
          return false;
        }
      return true;
    }
    bool layer::finished (const external_id_type& id, const result_type& result)
    {
      try
      {
        internal_id_type int_id (map_to_internal (id));
        {
          lock_t lock(mutex_);
          descriptor_ptr desc (lookup (int_id));
          {
            desc->output (we::mgmt::type::activity_t (result).output());

            DLOG (TRACE, "finished" << " (" << desc->name() << ")-" << id);
          }
        }

        post_finished_notification (int_id);
      }
      catch (const std::exception&)
      {
        return false;
      }

      return true;
    }
    bool layer::failed ( const external_id_type& id
                       , const result_type& result
                       , const int error_code
                       , const std::string& error_message
                       )
    {
      fhg::util::remove_unused_variable_warning (result);

      try
      {
        descriptor_ptr d = lookup (map_to_internal (id));

        d->set_error_code (error_code);
        d->set_error_message (d->name () + ": " + error_message);
        d->set_result (result);

        MLOG (TRACE, "failed ( " << id << " ) := " << error_message);

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
        post_failed_notification (d->id ());
        return true;
      }
      catch (const std::exception &ex)
      {
        MLOG (ERROR, "could not mark activity as failed: " << ex.what ());
        throw;
        //          return false;
      }
    }
    bool layer::cancelled (const external_id_type& id)
    {
      DLOG(TRACE, "cancelled (" << id << ")");

      try
      {
        post_cancelled_notification (map_to_internal (id));
        return true;
      }
      catch (const std::exception&)
      {
        DMLOG (WARN, "tried to notify cancelled for unknown activity " << id);

        return false;
      }
    }
  }
}
