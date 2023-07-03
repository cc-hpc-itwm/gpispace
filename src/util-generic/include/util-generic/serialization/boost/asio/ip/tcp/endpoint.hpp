// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
