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
