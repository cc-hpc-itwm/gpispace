#include "ns_uuid.hpp"
#include "header.hpp"

#include <cstring>

#include <boost/lexical_cast.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <boost/thread.hpp>

namespace fhg
{
  namespace com
  {
    namespace p2p
    {
      static boost::uuids::uuid fhg_com_uuid ()
      {
        static boost::uuids::string_generator g;
        static boost::uuids::uuid u = g(FHG_COM_NS_UUID);
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
            boost::lock_guard<boost::mutex> lock(m_mutex);
            return m_gen (name);
          }
        private:
          mutable boost::mutex m_mutex;
          boost::uuids::name_generator m_gen;
        };
      }

      static boost::uuids::uuid fhg_com_gen_uuid(std::string const & name)
      {
        static detail::fhg_com_gen_uuid gen;
        return gen(name);
      }

      void translate_name (std::string const & name, address_t & addr)
      {
        boost::uuids::uuid u = fhg_com_gen_uuid(name);
        memcpy (&addr, &u, sizeof(u));
      }

      address_t::address_t ()
      {
        memset (data, 0, sizeof (data));
      }

      address_t::address_t (std::string const &nm)
      {
        translate_name (nm, *this);
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
      bool operator<(address_t const& lhs, address_t const& rhs)
      {
        return memcmp (&lhs, &rhs, sizeof(address_t)) < 0;
      }
      bool operator>(address_t const& lhs, address_t const& rhs)
      {
        return memcmp (&lhs, &rhs, sizeof(address_t)) > 0;
      }
      bool operator<=(address_t const& lhs, address_t const& rhs)
      {
        return ! (lhs > rhs);
      }
      bool operator>=(address_t const& lhs, address_t const& rhs)
      {
        return ! (lhs < rhs);
      }

      void swap(address_t& lhs, address_t& rhs)
      {
        uint8_t tmp[sizeof(address_t)];
        memcpy (tmp, &lhs, sizeof(address_t));
        memcpy (&lhs, &rhs, sizeof(address_t));
        memcpy (&rhs, tmp, sizeof(address_t));
      }

      std::size_t hash_value(address_t const& a)
      {
        boost::uuids::uuid u;
        memcpy (&u, &a, sizeof(boost::uuids::uuid));
        return hash_value (u);
      }

      std::string to_string (address_t const & a)
      {
        boost::uuids::uuid u;
        memcpy (&u, &a, sizeof(boost::uuids::uuid));
        return boost::lexical_cast<std::string>(u);
      }

      std::ostream & operator<<(std::ostream & os, address_t const &a)
      {
        return os << to_string (a);
      }
    }
  }
}
