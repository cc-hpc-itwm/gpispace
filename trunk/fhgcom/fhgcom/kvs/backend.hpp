#ifndef FHG_COM_KVS_BACKEND_HPP
#define FHG_COM_KVS_BACKEND_HPP 1

#include <fhglog/fhglog.hpp>

namespace fhg
{
  namespace com
  {
    namespace kvs
    {
      namespace detail
      {
        class null_backend
        {
        public:
          typedef std::size_t size_type;
          typedef std::string key_type;
          typedef std::string value_type;

          explicit
          null_backend ()
          {}

          template <typename Val>
          void put (key_type const &, Val )
          {
          }

          template <typename T>
          T get (key_type const & k) const
          {
            throw std::runtime_error ("not found: " + k);
          }

          std::string const & get(key_type const & k) const
          {
            throw std::runtime_error ("not found: " + k);
          }

          size_type size () const { return 0; }
          size_type max_size () const { return 0; }
          void max_size (const size_type) {}
        };
      }
    }
  }
}

#endif
