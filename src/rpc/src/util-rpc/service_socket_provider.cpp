// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-rpc/detail/service_stream_provider.ipp>
#include <util-rpc/service_socket_provider.hpp>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      template struct service_stream_provider
        <::boost::asio::local::stream_protocol, service_socket_provider_traits>;
      template struct service_stream_provider_with_deferred_start
        <::boost::asio::local::stream_protocol, service_socket_provider_traits>;
      template struct service_stream_provider_with_deferred_dispatcher
        <::boost::asio::local::stream_protocol, service_socket_provider_traits>;
    }
  }
}
