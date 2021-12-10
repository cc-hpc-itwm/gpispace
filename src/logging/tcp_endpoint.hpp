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

#include <gspc/detail/dllexport.hpp>

#include <boost/any.hpp>

#include <string>
#include <utility>
#include <vector>

namespace fhg
{
  namespace logging
  {
    namespace error
    {
      struct GSPC_DLLEXPORT bad_host_and_port_string : std::invalid_argument
      {
        bad_host_and_port_string (std::string);
      };
    }

    //! \todo Actually part of fhg::rpc.
    struct GSPC_DLLEXPORT tcp_endpoint
    {
      std::string host;
      unsigned short port;

      tcp_endpoint (std::string host, unsigned short port);
      tcp_endpoint (std::string const& host_and_port);
      tcp_endpoint (std::pair<std::string, unsigned short> host_and_port);

      std::string to_string() const;

      tcp_endpoint() = default;
      tcp_endpoint (tcp_endpoint const&) = default;
      tcp_endpoint (tcp_endpoint&&) = default;
      tcp_endpoint& operator= (tcp_endpoint const&) = default;
      tcp_endpoint& operator= (tcp_endpoint&&) = default;
      ~tcp_endpoint() = default;

      template<typename Archive>
        void serialize (Archive&, tcp_endpoint&, unsigned int);

      operator std::pair<std::string, unsigned short>() const;
    };

    GSPC_DLLEXPORT void validate
      (::boost::any&, std::vector<std::string> const&, tcp_endpoint*, int);
  }
}

#include <logging/tcp_endpoint.ipp>
