// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <util-rpc/detail/service_stream_provider.hpp>

#include <boost/asio/local/stream_protocol.hpp>
#include <boost/mpl/vector.hpp>

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
