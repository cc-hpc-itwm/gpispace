// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <boost/asio/local/stream_protocol.hpp>
#include <boost/serialization/split_free.hpp>

#include <string>

namespace boost
{
  namespace serialization
  {
    template<typename Archive>
      void load ( Archive& ar
                , ::boost::asio::local::stream_protocol::endpoint& ep
                , const unsigned int
                )
    {
      std::string path;
      ar & path;
      ep = ::boost::asio::local::stream_protocol::endpoint (path);
    }
    template<typename Archive>
      void save ( Archive& ar
                , ::boost::asio::local::stream_protocol::endpoint const& ep
                , const unsigned int
                )
    {
      std::string const path (ep.path());
      ar & path;
    }
  }
}

BOOST_SERIALIZATION_SPLIT_FREE (::boost::asio::local::stream_protocol::endpoint)
