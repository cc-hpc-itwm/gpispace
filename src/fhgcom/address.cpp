// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
