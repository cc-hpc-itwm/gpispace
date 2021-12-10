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
