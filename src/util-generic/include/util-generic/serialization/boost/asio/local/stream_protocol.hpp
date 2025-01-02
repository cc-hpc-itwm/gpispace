// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
