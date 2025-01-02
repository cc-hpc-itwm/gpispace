// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-rpc/detail/service_stream_provider.hpp>

#include <boost/asio/ip/tcp.hpp>

namespace fhg
{
  namespace rpc
  {
    struct service_tcp_provider_traits
    {
      template<typename Socket>
        static void apply_socket_options (Socket& socket)
      {
        socket.set_option (::boost::asio::ip::tcp::no_delay (true));
      }
    };

    //! A scoped service provider communicating via TCP/IP. The
    //! endpoint is a host+port pair which can be specified as pair or
    //! separate.
    //!
    //! See \c detail::service_stream_provider_with_deferred_start for
    //! documentation of constructors and generic usage hints.
    //!
    //! \see detail::service_stream_provider,
    //! detail::service_stream_provider_with_deferred_start,
    //! remote_tcp_endpoint
    using service_tcp_provider
      = detail::service_stream_provider
          < ::boost::asio::ip::tcp
          , service_tcp_provider_traits
          >;

    //! A \c service_tcp_provider variant which allows to fork between
    //! setting up infrastructure and start of call handling.
    //! \see detail::service_stream_provider_with_deferred_start,
    //! service_tcp_provider
    using service_tcp_provider_with_deferred_start
      = detail::service_stream_provider_with_deferred_start
          < ::boost::asio::ip::tcp
          , service_tcp_provider_traits
          >;

    //! A \c service_tcp_provider variant which allows to defer
    //! creation of a \c service_dispatcher, stalling calls until it
    //! is provided.
    //! \see detail::service_stream_provider_with_deferred_dispatcher,
    //! service_tcp_provider
    using service_tcp_provider_with_deferred_dispatcher
      = detail::service_stream_provider_with_deferred_dispatcher
          < ::boost::asio::ip::tcp
          , service_tcp_provider_traits
          >;

    namespace detail
    {
      extern template struct service_stream_provider
        <::boost::asio::ip::tcp, service_tcp_provider_traits>;
      extern template struct service_stream_provider_with_deferred_start
        <::boost::asio::ip::tcp, service_tcp_provider_traits>;
      extern template struct service_stream_provider_with_deferred_dispatcher
        <::boost::asio::ip::tcp, service_tcp_provider_traits>;
    }
  }
}
