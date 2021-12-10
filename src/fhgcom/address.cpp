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

#include <fhgcom/address.hpp>

#include <boost/uuid/uuid_generators.hpp>

#include <mutex>

namespace fhg
{
  namespace com
  {
    namespace p2p
    {
      static ::boost::uuids::uuid fhg_com_uuid (std::string const& name)
      {
        static ::boost::uuids::name_generator m_gen
          { ::boost::uuids::string_generator{}
              ("c9fe00cb-d9f7-432e-9235-66b7929b6e2a")
          };
        static std::mutex m_mutex;
        std::lock_guard<std::mutex> lock {m_mutex};
        return m_gen (name);
      }

      address_t::address_t (host_t const& host, port_t const& port)
        : _uuid {fhg_com_uuid (std::string {host} + ":" + to_string (port))}
      {}

      // standard operators
      bool operator==(address_t const& lhs, address_t const& rhs)
      {
        return lhs._uuid == rhs._uuid;
      }
      bool operator!=(address_t const& lhs, address_t const& rhs)
      {
        return ! (lhs == rhs);
      }

      bool operator< (address_t const& lhs, address_t const& rhs)
      {
        return lhs._uuid < rhs._uuid;
      }

      std::size_t hash_value (address_t const& address)
      {
        return std::hash<address_t>() (address);
      }
    }
  }
}

namespace std
{
  size_t hash<fhg::com::p2p::address_t>::operator()
    (fhg::com::p2p::address_t const& a) const
  {
    return hash_value (a._uuid);
  }
}
