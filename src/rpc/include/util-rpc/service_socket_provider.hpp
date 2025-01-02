// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-rpc/detail/service_stream_provider.hpp>

#include <boost/asio/local/stream_protocol.hpp>

namespace fhg
{
  namespace rpc
  {
    struct service_socket_provider_traits
    {
      template<typename Socket>
        static void apply_socket_options (Socket&) {}
    };

    //! A scoped service provider using UNIX domain sockets. The
    //! endpoint is a \c std::string.
    //!
    //! See \c detail::service_stream_provider_with_deferred_start for
    //! documentation of constructors and generic usage hints.
    //!
    //! \note The path is not guaranteed to be a filesystem path but
    //! may be an "abstract" socket address which is indicated by the
    //! first character being `'\0'`. Because of this, do not use
    //! C-string APIs to process or pass the endpoint around.
    //!
    //! \see detail::service_stream_provider,
    //! detail::service_stream_provider_with_deferred_start,
    //! remote_socket_endpoint, manpage unix(7)
    using service_socket_provider
      = detail::service_stream_provider
          < ::boost::asio::local::stream_protocol
          , service_socket_provider_traits
          >;

    //! A \c service_socket_provider variant which allows to fork
    //! between setting up infrastructure and start of call handling.
    //! \see detail::service_stream_provider_with_deferred_start,
    //! service_socket_provider
    using service_socket_provider_with_deferred_start
      = detail::service_stream_provider_with_deferred_start
          < ::boost::asio::local::stream_protocol
          , service_socket_provider_traits
          >;

    //! A \c service_socket_provider variant which allows to defer
    //! creation of a \c service_dispatcher, stalling calls until it
    //! is provided.
    //! \see detail::service_stream_provider_with_deferred_dispatcher,
    //! service_socket_provider
    using service_socker_provider_with_deferred_dispatcher
      = detail::service_stream_provider_with_deferred_dispatcher
          < ::boost::asio::local::stream_protocol
          , service_socket_provider_traits
          >;

    namespace detail
    {
      extern template struct service_stream_provider
        <::boost::asio::local::stream_protocol, service_socket_provider_traits>;
      extern template struct service_stream_provider_with_deferred_start
        <::boost::asio::local::stream_protocol, service_socket_provider_traits>;
      extern template struct service_stream_provider_with_deferred_dispatcher
        <::boost::asio::local::stream_protocol, service_socket_provider_traits>;
    }
  }
}
