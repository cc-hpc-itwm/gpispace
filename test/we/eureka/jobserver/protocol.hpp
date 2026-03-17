// Copyright (C) 2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <test/we/eureka/jobserver/EurekaGroup.hpp>
#include <test/we/eureka/jobserver/Task.hpp>

#include <gspc/rpc/function_description.hpp>






          namespace gspc::we::test::eureka::jobserver::protocol
          {
            FHG_RPC_FUNCTION_DESCRIPTION
              (running, void (Task, EurekaGroup));

            FHG_RPC_FUNCTION_DESCRIPTION
              (cancelled, void (Task, EurekaGroup));

            FHG_RPC_FUNCTION_DESCRIPTION
              (exited_or_cancelled, void (Task, EurekaGroup));
          }
