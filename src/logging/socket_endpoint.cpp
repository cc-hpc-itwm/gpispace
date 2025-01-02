// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <logging/socket_endpoint.hpp>

#include <util-generic/boost/program_options/validators.hpp>
#include <util-generic/hostname.hpp>

#include <exception>

namespace fhg
{
  namespace logging
  {
    namespace error
    {
      bad_host_and_socket_string::bad_host_and_socket_string (std::string str)
        : std::invalid_argument
            ("bad host and socket '" + str + "': expected 'host:socket'")
      {}
    }

    socket_endpoint::socket_endpoint (Socket local_socket)
      : socket_endpoint (fhg::util::hostname(), local_socket)
    {}

    socket_endpoint::socket_endpoint (std::string host_, Socket socket_)
      : host (std::move (host_))
      , socket (std::move (socket_))
    {}

    namespace
    {
      std::pair<std::string, socket_endpoint::Socket> split
        (std::string const& raw)
      try
      {
        auto const colon_pos (raw.find (':'));

#define REQUIRE(cond_)                                          \
        do                                                      \
        {                                                       \
          if (!(cond_)) { throw std::runtime_error (#cond_); }  \
        } while (false)

        REQUIRE (colon_pos != std::string::npos);
        REQUIRE (colon_pos + 1 != raw.size());
        REQUIRE (colon_pos != 0);

        auto const host (raw.substr (0, colon_pos));
        auto socket (raw.substr (colon_pos + 1));

        if (socket.size() >= 2 && socket[0] == '\\' && socket[1] == '0')
        {
          socket = std::string (1, '\0') + (socket.data() + 2);
        }

#undef REQUIRE

        return {std::move (host), std::move (socket)};
      }
      catch (...)
      {
        std::throw_with_nested (error::bad_host_and_socket_string (raw));
      }
    }

    socket_endpoint::socket_endpoint (std::string const& host_and_socket)
      : socket_endpoint (split (host_and_socket))
    {}

    socket_endpoint::socket_endpoint (std::pair<std::string, Socket> pair)
      : socket_endpoint (std::move (pair.first), std::move (pair.second))
    {}

    std::string socket_endpoint::to_string() const
    {
      auto path (socket.path());
      if (path.size() >= 1 && path[0] == '\0')
      {
        path = "\\0" + std::string (path.data() + 1);
      }

      return host + ":" + path;
    }

    socket_endpoint::operator std::pair<std::string, Socket>() const
    {
      return {host, socket};
    }

    void validate ( ::boost::any& result
                  , std::vector<std::string> const& values
                  , socket_endpoint*
                  , int
                  )
    {
      return util::boost::program_options::validate<socket_endpoint>
        (result, values);
    }
  }
}
