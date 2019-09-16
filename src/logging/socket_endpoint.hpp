#pragma once

#include <util-generic/serialization/boost/asio/local/stream_protocol.hpp>
#include <util-generic/serialization/by_member.hpp>

#include <boost/any.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/serialization/utility.hpp>

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

      operator std::pair<std::string, Socket>() const;
    };

    void validate
      (boost::any&, std::vector<std::string> const&, socket_endpoint*, int);
  }
}

FHG_UTIL_SERIALIZATION_BY_MEMBER ( fhg::logging::socket_endpoint
                                 , &fhg::logging::socket_endpoint::host
                                 , &fhg::logging::socket_endpoint::socket
                                 )
