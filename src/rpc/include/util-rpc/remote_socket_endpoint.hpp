// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-rpc/detail/remote_stream_endpoint.hpp>

#include <boost/asio/local/stream_protocol.hpp>

namespace fhg
{
  namespace rpc
  {
    struct remote_socket_endpoint_traits
    {
      template<typename Socket>
        static void apply_socket_options (Socket&) {}
    };

    //! A remote endpoint communicating via a UNIX stream socket. The
    //! endpoint is a \c std::string.
    //!
    //! See \c detail::remote_stream_endpoint for documentation of
    //! constructors and generic usage hints.
    //!
    //! \note The path is not guaranteed to be a filesystem path but
    //! may be an "abstract" socket address which is indicated by the
    //! first character being `'\0'`. Because of this, do not use
    //! C-string APIs to process or pass the endpoint around.
    //!
    //! \see service_socket_provider, manpage unix(7)
    using remote_socket_endpoint
      = detail::remote_stream_endpoint
          <::boost::asio::local::stream_protocol, remote_socket_endpoint_traits>;

    namespace detail
    {
      extern template struct remote_stream_endpoint
        <::boost::asio::local::stream_protocol, remote_socket_endpoint_traits>;
    }
  }
}
