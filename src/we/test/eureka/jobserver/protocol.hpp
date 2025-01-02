// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/test/eureka/jobserver/EurekaGroup.hpp>
#include <we/test/eureka/jobserver/Task.hpp>

#include <util-rpc/function_description.hpp>

namespace gspc
{
  namespace we
  {
    namespace test
    {
      namespace eureka
      {
        namespace jobserver
        {
          namespace protocol
          {
            FHG_RPC_FUNCTION_DESCRIPTION
              (running, void (Task, EurekaGroup));

            FHG_RPC_FUNCTION_DESCRIPTION
              (cancelled, void (Task, EurekaGroup));

            FHG_RPC_FUNCTION_DESCRIPTION
              (exited_or_cancelled, void (Task, EurekaGroup));
          }
        }
      }
    }
  }
}
