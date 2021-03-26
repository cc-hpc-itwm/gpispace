// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <sdpa/events/SDPAEvent.hpp>

#include <functional>

namespace sdpa
{
  namespace events
  {
    class delayed_function_call : public sdpa::events::SDPAEvent
    {
    public:
      delayed_function_call (std::function<void()> function)
        : SDPAEvent()
        , _function (function)
      {}

      virtual void handleBy
        (fhg::com::p2p::address_t const&, EventHandler*) override
      {
        _function();
      }

    private:
      std::function<void()> _function;
    };

    //! \note No serialization: Shall only be used within daemon, not over net
  }
}
