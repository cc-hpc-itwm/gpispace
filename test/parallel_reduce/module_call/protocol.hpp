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

#include <test/parallel_reduce/module_call/Task.hpp>

#include <util-rpc/function_description.hpp>

#include <boost/serialization/utility.hpp>

#include <string>
#include <utility>

namespace gspc
{
  namespace test
  {
    namespace parallel_reduce
    {
      namespace module_call
      {
        namespace protocol
        {
          // module -> jobserver
          FHG_RPC_FUNCTION_DESCRIPTION
            ( running
            , void ( Task
                   , std::string worker_name
                   , std::pair<std::string, unsigned short> release_address
                   )
            );

          // jobserver -> module
          FHG_RPC_FUNCTION_DESCRIPTION
            ( release
            , void ( Task
                   , std::string worker_name
                   )
            );
        }
      }
    }
  }
}
