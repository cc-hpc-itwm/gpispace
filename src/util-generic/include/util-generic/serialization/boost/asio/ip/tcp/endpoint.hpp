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

#include <boost/asio/ip/tcp.hpp>
#include <boost/serialization/split_free.hpp>

#include <string>

namespace boost
{
  namespace serialization
  {
    template<typename Archive>
      void load ( Archive& ar
                , ::boost::asio::ip::tcp::endpoint& ep
                , const unsigned int
                )
    {
      std::string address;
      unsigned short port;
      ar & address;
      ar & port;
      ep = ::boost::asio::ip::tcp::endpoint
        (::boost::asio::ip::address::from_string (address), port);
    }
    template<typename Archive>
      void save ( Archive& ar
                , ::boost::asio::ip::tcp::endpoint const& ep
                , const unsigned int
                )
    {
      std::string const address (ep.address().to_string());
      unsigned short const port (ep.port());
      ar & address;
      ar & port;
    }
  }
}

BOOST_SERIALIZATION_SPLIT_FREE (::boost::asio::ip::tcp::endpoint)
