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

      using JobEvent::JobEvent;

      virtual void handleBy (std::string const&, EventHandler*) override
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
