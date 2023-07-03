// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/uuid/uuid.hpp>

#include <fhgcom/peer_info.hpp>

#include <functional>

namespace fhg
{
  namespace com
  {
    namespace p2p
    {
      struct address_t
      {
        explicit address_t() = default;

        //! \todo store host, port and let clients observe them!?
        explicit address_t (host_t const&, port_t const&);

        ::boost::uuids::uuid _uuid;
      };

      static_assert (16ul == sizeof (address_t), "size of fhg::com::p2p::address_t");

      // standard operators
      bool operator==(address_t const& lhs, address_t const& rhs);
      bool operator!=(address_t const& lhs, address_t const& rhs);
      bool operator< (address_t const& lhs, address_t const& rhs);

      std::size_t hash_value (address_t const& address);
    }
  }
}

namespace std
{
  template<> struct hash<fhg::com::p2p::address_t>
  {
    size_t operator()(fhg::com::p2p::address_t const&) const;
  };
}
