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

#include <boost/any.hpp>
#include <boost/asio/local/stream_protocol.hpp>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace fhg
{
  namespace logging
  {
    namespace error
    {
      struct bad_host_and_socket_string : std::invalid_argument
      {
        bad_host_and_socket_string (std::string);
      };
    }

    //! \todo Actually part of fhg::rpc.
    struct socket_endpoint
    {
      using Socket = boost::asio::local::stream_protocol::endpoint;
      std::string host;
      Socket socket;

      socket_endpoint (Socket local_socket);
      socket_endpoint (std::string host, Socket socket);
      socket_endpoint (std::string const& host_and_socket);
      socket_endpoint (std::pair<std::string, Socket> host_and_socket);

      std::string to_string() const;

      socket_endpoint() = default;
      socket_endpoint (socket_endpoint const&) = default;
      socket_endpoint (socket_endpoint&&) = default;
      socket_endpoint& operator= (socket_endpoint const&) = default;
      socket_endpoint& operator= (socket_endpoint&&) = default;
      ~socket_endpoint() = default;

      template<typename Archive>
        void serialize (Archive&, socket_endpoint&, unsigned int);

      operator std::pair<std::string, Socket>() const;
    };

    void validate
      (boost::any&, std::vector<std::string> const&, socket_endpoint*, int);
  }
}

#include <logging/socket_endpoint.ipp>
