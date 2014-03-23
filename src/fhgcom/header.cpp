#include <fhgcom/header.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <cstring>

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

      address_t::address_t (std::string const &name)
      {
        translate_name (name, *this);
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
    }
  }
}
