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

#include <boost/asio/ip/address.hpp>

#include <string>
#include <utility>

namespace fhg
{
  namespace util
  {
    //! Convert the given Boost.Asio IP \a address into a string that
    //! can be used to connect from remote machines.
    //! \note If given an address referring to the local host, assumes
    //! to be called on that host, i.e. should always be called in the
    //! same process creating the address.
    std::string connectable_to_address_string
      (::boost::asio::ip::address const& address);

    //! Convert a Boost.Asio IP \a endpoint into a hostname string and
    //! port pair that can be used to connect from remote machines.
    //! \note If given an endpoint referring to the local host,
    //! assumes to be called on that host, i.e. should always be
    //! called in the same process creating the endpoint.
    template<typename BoostAsioIpEndpoint>
      std::pair<std::string, unsigned short>
        connectable_to_address_string (BoostAsioIpEndpoint endpoint);
  }
}

#include <util-generic/connectable_to_address_string.ipp>
