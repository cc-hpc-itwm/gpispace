// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <fhgcom/header.hpp>
#include <fhgcom/peer_info.hpp>

#include <boost/optional.hpp>

#include <forward_list>
#include <string>

namespace sdpa
{
  struct master_network_info
  {
    master_network_info (std::string const& host_, std::string const& port_)
      : host (host_)
      , port (port_)
      , address (boost::none)
    {}

    fhg::com::host_t host;
    fhg::com::port_t port;
    boost::optional<fhg::com::p2p::address_t> address;
  };

  using master_info_t = std::forward_list<master_network_info>;
}
