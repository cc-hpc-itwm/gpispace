// Copyright (C) 2018-2019,2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <boost/any.hpp>
#include <boost/asio/local/stream_protocol.hpp>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


  namespace gspc::logging
  {
    namespace error
    {
      struct GSPC_EXPORT bad_host_and_socket_string : std::invalid_argument
      {
        bad_host_and_socket_string (std::string);
      };
    }

    //! \todo Actually part of gspc::rpc.
    struct GSPC_EXPORT socket_endpoint
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

    GSPC_EXPORT void validate
      (::boost::any&, std::vector<std::string> const&, socket_endpoint*, int);
  }


#include <gspc/logging/socket_endpoint.ipp>
