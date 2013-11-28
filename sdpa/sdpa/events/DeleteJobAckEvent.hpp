#ifndef SDPA_DeleteJobAckEvent_HPP
#define SDPA_DeleteJobAckEvent_HPP 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class DeleteJobAckEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<DeleteJobAckEvent> Ptr;

      DeleteJobAckEvent ( const address_t& a_from
                        , const address_t& a_to
                        , const sdpa::job_id_t& a_job_id
                        )
        :  sdpa::events::JobEvent( a_from, a_to, a_job_id)
      {}

      std::string str() const
      {
        return "DeleteJobAckEvent(" + job_id().str () + ")";
      }

      virtual void handleBy(EventHandler* handler)
      {
        throw std::runtime_error
          ( "This method should never be called as only the client should handle"
            " this event and it does not use the EventHandler interface."
          );
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (DeleteJobAckEvent)
  }
}

#endif
