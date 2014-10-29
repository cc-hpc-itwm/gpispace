#ifndef SDPA_MGMT_EVENT_HPP
#define SDPA_MGMT_EVENT_HPP 1

#include <sdpa/events/SDPAEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class MgmtEvent : public sdpa::events::SDPAEvent
    {
    public:
      MgmtEvent (const address_t &a_from)
        : SDPAEvent (a_from)
      {}
    };
  }
}

#endif
