// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
