// Copyright (C) 2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <test/parallel_reduce/module_call/Task.hpp>

#include <gspc/rpc/function_description.hpp>

#include <boost/serialization/utility.hpp>

#include <string>
#include <utility>





        namespace gspc::test::parallel_reduce::module_call::protocol
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
