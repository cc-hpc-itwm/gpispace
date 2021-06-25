// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <rpc/detail/service_stream_provider.hpp>

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
        socket.set_option (boost::asio::ip::tcp::no_delay (true));
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
          < boost::asio::ip::tcp
          , service_tcp_provider_traits
          >;

    //! A \c service_tcp_provider variant which allows to fork between
    //! setting up infrastructure and start of call handling.
    //! \see detail::service_stream_provider_with_deferred_start,
    //! service_tcp_provider
    using service_tcp_provider_with_deferred_start
      = detail::service_stream_provider_with_deferred_start
          < boost::asio::ip::tcp
          , service_tcp_provider_traits
          >;

    //! A \c service_tcp_provider variant which allows to defer
    //! creation of a \c service_dispatcher, stalling calls until it
    //! is provided.
    //! \see detail::service_stream_provider_with_deferred_dispatcher,
    //! service_tcp_provider
    using service_tcp_provider_with_deferred_dispatcher
      = detail::service_stream_provider_with_deferred_dispatcher
          < boost::asio::ip::tcp
          , service_tcp_provider_traits
          >;

    namespace detail
    {
      extern template struct service_stream_provider
        <boost::asio::ip::tcp, service_tcp_provider_traits>;
      extern template struct service_stream_provider_with_deferred_start
        <boost::asio::ip::tcp, service_tcp_provider_traits>;
      extern template struct service_stream_provider_with_deferred_dispatcher
        <boost::asio::ip::tcp, service_tcp_provider_traits>;
    }
  }
}
