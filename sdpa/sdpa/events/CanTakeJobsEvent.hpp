#ifndef SDPA_CAN_TAKE_JOBS_EVENT_HPP
#define SDPA_CAN_TAKE_JOBS_EVENT_HPP 1

#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class CanTakeJobsEvent : public MgmtEvent
    {
    public:
      typedef boost::shared_ptr<CanTakeJobsEvent> Ptr;

      CanTakeJobsEvent() : MgmtEvent(){}

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleCanTakeJobsEvent (source, this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_MGMTEVENT_OVERLOAD (CanTakeJobsEvent);
  }
}

#endif
