#ifndef SDPA_WORKER_REGISTRATION_ACK_EVENT_HPP
#define SDPA_WORKER_REGISTRATION_ACK_EVENT_HPP 1

#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class WorkerRegistrationAckEvent : public MgmtEvent
    {
    public:
      typedef boost::shared_ptr<WorkerRegistrationAckEvent> Ptr;

      WorkerRegistrationAckEvent ( const address_t& a_from
                                 , const address_t& a_to
                                 )
        : MgmtEvent (a_from, a_to)
      {}

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleWorkerRegistrationAckEvent (this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_MGMTEVENT_OVERLOAD (WorkerRegistrationAckEvent)
  }
}

#endif
