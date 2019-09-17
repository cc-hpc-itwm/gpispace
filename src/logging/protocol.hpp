#pragma once

#include <logging/endpoint.hpp>
#include <logging/message.hpp>
#include <logging/socket_endpoint.hpp>
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
      FHG_RPC_FUNCTION_DESCRIPTION (register_receiver, void (endpoint));
      FHG_RPC_FUNCTION_DESCRIPTION
        (register_socket_receiver, void (socket_endpoint));
      FHG_RPC_FUNCTION_DESCRIPTION
        (register_tcp_receiver, void (tcp_endpoint));
    }
  }
}
