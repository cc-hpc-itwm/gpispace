#pragma once

#include <sdpa/events/SDPAEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class MgmtEvent : public sdpa::events::SDPAEvent
    {
    public:
      using SDPAEvent::SDPAEvent;
    };
  }
}
