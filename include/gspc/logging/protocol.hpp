// Copyright (C) 2018-2019,2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/logging/endpoint.hpp>
#include <gspc/logging/message.hpp>

#include <gspc/rpc/function_description.hpp>

#include <boost/serialization/vector.hpp>



    namespace gspc::logging::protocol
    {
      FHG_RPC_FUNCTION_DESCRIPTION (receive, void (message));
      FHG_RPC_FUNCTION_DESCRIPTION (register_receiver, void (endpoint));

      namespace receiver
      {
        FHG_RPC_FUNCTION_DESCRIPTION
          (add_emitters, void (std::vector<endpoint>));
      }
    }
