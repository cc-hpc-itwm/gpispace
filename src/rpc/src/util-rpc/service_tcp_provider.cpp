// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-rpc/detail/service_stream_provider.ipp>
#include <util-rpc/service_tcp_provider.hpp>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      template struct service_stream_provider
        <::boost::asio::ip::tcp, service_tcp_provider_traits>;
      template struct service_stream_provider_with_deferred_start
        <::boost::asio::ip::tcp, service_tcp_provider_traits>;
      template struct service_stream_provider_with_deferred_dispatcher
        <::boost::asio::ip::tcp, service_tcp_provider_traits>;
    }
  }
}
