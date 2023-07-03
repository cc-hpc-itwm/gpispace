// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-rpc/detail/remote_stream_endpoint.ipp>
#include <util-rpc/remote_socket_endpoint.hpp>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      template struct remote_stream_endpoint
        <::boost::asio::local::stream_protocol, remote_socket_endpoint_traits>;
    }
  }
}
