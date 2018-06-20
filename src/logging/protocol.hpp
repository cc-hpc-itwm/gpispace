#pragma once

#include <logging/message.hpp>
#include <logging/tcp_endpoint.hpp>

#include <rpc/function_description.hpp>

namespace fhg
{
  namespace logging
  {
    namespace protocol
    {
      FHG_RPC_FUNCTION_DESCRIPTION
        (receive, void (message));
      FHG_RPC_FUNCTION_DESCRIPTION
        (register_tcp_receiver, void (tcp_endpoint));
    };
  }
}
