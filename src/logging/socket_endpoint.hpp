// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

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
      struct GSPC_DLLEXPORT bad_host_and_socket_string : std::invalid_argument
      {
        bad_host_and_socket_string (std::string);
      };
    }

    //! \todo Actually part of fhg::rpc.
    struct GSPC_DLLEXPORT socket_endpoint
    {
      using Socket = ::boost::asio::local::stream_protocol::endpoint;
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

    GSPC_DLLEXPORT void validate
      (::boost::any&, std::vector<std::string> const&, socket_endpoint*, int);
  }
}

#include <logging/socket_endpoint.ipp>
