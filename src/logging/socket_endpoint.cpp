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
