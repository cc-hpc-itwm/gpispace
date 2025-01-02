// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
