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

#include <fhgcom/header.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <functional>
#include <mutex>

namespace fhg
{
  namespace com
  {
    namespace p2p
    {
      static boost::uuids::uuid fhg_com_uuid ()
      {
        static boost::uuids::string_generator g;
        static boost::uuids::uuid u = g("c9fe00cb-d9f7-432e-9235-66b7929b6e2a");
        return u;
      }

      namespace detail
      {
        struct fhg_com_gen_uuid
        {
          fhg_com_gen_uuid ()
            : m_gen (fhg_com_uuid())
          {}

          boost::uuids::uuid operator () (std::string const & name)
          {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_gen (name);
          }
        private:
          std::mutex m_mutex;
          boost::uuids::name_generator m_gen;
        };
      }

      address_t::address_t ()
      {
        memset (data, 0, sizeof (data));
      }

      address_t::address_t (std::string const &name)
      {
        static detail::fhg_com_gen_uuid gen;
        boost::uuids::uuid u = gen(name);
        memcpy (this, &u, sizeof(u));
      }

      // standard operators
      bool operator==(address_t const& lhs, address_t const& rhs)
      {
        return memcmp (&lhs, &rhs, sizeof(address_t)) == 0;
      }
      bool operator!=(address_t const& lhs, address_t const& rhs)
      {
        return ! (lhs == rhs);
      }

      bool operator< (address_t const& lhs, address_t const& rhs)
      {
        return lhs.data < rhs.data;
      }

      std::string to_string (address_t const & a)
      {
        boost::uuids::uuid u;
        memcpy (&u, &a, sizeof(boost::uuids::uuid));
        return boost::lexical_cast<std::string>(u);
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
    boost::uuids::uuid u;
    memcpy (&u, &a, sizeof (boost::uuids::uuid));
    return hash_value (u);
  }
}
