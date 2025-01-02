// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
