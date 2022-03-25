// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <logging/endpoint.hpp>
#include <logging/message.hpp>

#include <util-rpc/function_description.hpp>

#include <boost/serialization/vector.hpp>

namespace fhg
{
  namespace logging
  {
    namespace protocol
    {
      FHG_RPC_FUNCTION_DESCRIPTION (receive, void (message));
      FHG_RPC_FUNCTION_DESCRIPTION (register_receiver, void (endpoint));

      namespace receiver
      {
        FHG_RPC_FUNCTION_DESCRIPTION
          (add_emitters, void (std::vector<endpoint>));
      }
    }
  }
}
