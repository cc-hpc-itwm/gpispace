#ifndef SDPA_INTERRUPTEVENT_HPP
#define SDPA_INTERRUPTEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class InterruptEvent : public MgmtEvent
    {
    public:
      typedef sdpa::shared_ptr<InterruptEvent> Ptr;

      InterruptEvent ( const address_t& a_from
                     , const address_t& a_to
                     )
        : MgmtEvent (a_from, a_to)
      {}

      std::string str() const
      {
        return "InterruptEvent";
      }

      virtual void handleBy(EventHandler *handler)
      {
        handler->handleInterruptEvent(this);
      }
    };
  }
}

#endif
