#ifndef SDPA_DELETE_JOB_EVENT_HPP
#define SDPA_DELETE_JOB_EVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class DeleteJobEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<DeleteJobEvent> Ptr;

      using JobEvent::JobEvent;

      virtual void handleBy
        (std::string const& source, EventHandler* handler) override
      {
        handler->handleDeleteJobEvent (source, this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (DeleteJobEvent)
  }
}

#endif
