// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace sdpa
{
  namespace events
  {
    class SDPAEvent;

    class Codec
    {
    public:
      template<typename Event, typename... Args>
        std::string encode (Args&&... args) const;

      std::string encode (sdpa::events::SDPAEvent const* e) const;
      sdpa::events::SDPAEvent* decode (std::string const& s) const;
    };
  }
}

#include <sdpa/events/Codec.ipp>
