// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/events/EventHandler.hpp>

#include <fhgcom/address.hpp>

#include <boost/serialization/access.hpp>
#include <boost/shared_ptr.hpp>

namespace sdpa
{
  namespace events
  {
    class SDPAEvent
    {
    public:
      using Ptr = ::boost::shared_ptr<SDPAEvent>;

      virtual ~SDPAEvent() = default;
      SDPAEvent (SDPAEvent const&) = delete;
      SDPAEvent (SDPAEvent&&) = default;
      SDPAEvent& operator= (SDPAEvent const&) = delete;
      SDPAEvent& operator= (SDPAEvent&&) = delete;

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler*) = 0;

    protected:
      SDPAEvent() = default;

    private:
      friend class ::boost::serialization::access;
      template <class Archive>
      void serialize (Archive &, unsigned int)
      {
      }
    };
  }
}

#include <sdpa/events/Serialization.hpp>
