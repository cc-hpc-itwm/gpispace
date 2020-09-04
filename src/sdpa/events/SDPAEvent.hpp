// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <sdpa/events/EventHandler.hpp>

#include <fhgcom/header.hpp>

#include <boost/serialization/access.hpp>
#include <boost/shared_ptr.hpp>

namespace sdpa
{
  namespace events
  {
    class SDPAEvent
    {
    public:
      typedef boost::shared_ptr<SDPAEvent> Ptr;

      virtual ~SDPAEvent() = default;

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler*) = 0;

    protected:
      SDPAEvent() = default;

    private:
      friend class boost::serialization::access;
      template <class Archive>
      void serialize (Archive &, unsigned int)
      {
      }
    };
  }
}

#include <sdpa/events/Serialization.hpp>
