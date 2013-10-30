#ifndef SDPA_DeleteJobAckEvent_HPP
#define SDPA_DeleteJobAckEvent_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class DeleteJobAckEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<DeleteJobAckEvent> Ptr;

      DeleteJobAckEvent()
        : JobEvent ("", "", "", message_id_type())
      {}

      DeleteJobAckEvent ( const address_t& a_from
                        , const address_t& a_to
                        , const sdpa::job_id_t& a_job_id
                        , const message_id_type& mid
                        )
        :  sdpa::events::JobEvent( a_from, a_to, a_job_id, mid )
      {}

      std::string str() const
      {
        return "DeleteJobAckEvent(" + job_id().str () + ")";
      }

      virtual void handleBy(EventHandler* handler)
      {
        handler->handleDeleteJobAckEvent (this);
      }
    };
  }
}

#endif
