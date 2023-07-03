// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
