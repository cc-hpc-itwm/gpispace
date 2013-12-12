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
      descriptor_ptr desc
        (new detail::descriptor (generate_internal_id(), act));
      desc->set_user_data (data);
      desc->came_from_external_as (id);
      desc->inject_input ();

      submit (desc);
    }
    void layer::cancel (const external_id_type& id, const reason_type& reason)
    {
      MLOG (WARN, "cancel ( " << id << " ) := " << reason);

      post_cancel_activity_notification (map_to_internal (id));
    }
    void layer::finished (const external_id_type& id, const result_type& result)
    {
      internal_id_type int_id (map_to_internal (id));
      {
        lock_t lock(mutex_);
        descriptor_ptr desc (lookup (int_id));
        {
          desc->output (we::mgmt::type::activity_t (result).output());
        }
      }

      post_finished_notification (int_id);
    }
    void layer::failed ( const external_id_type& id
                       , const result_type& result
                       , const int error_code
                       , const std::string& error_message
                       )
    {
      descriptor_ptr d = lookup (map_to_internal (id));

      d->set_error_code (error_code);
      d->set_error_message (d->name () + ": " + error_message);
      d->set_result (result);

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
    }
    void layer::canceled (const external_id_type& id)
    {
      post_canceled_notification (map_to_internal (id));
    }
  }
}
