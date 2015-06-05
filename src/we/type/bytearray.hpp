#pragma once

#include <boost/type_traits/is_pointer.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/serialization/vector.hpp>

#include <algorithm>
#include <vector>

#include <iosfwd>
#include <sstream>
#include <string>

namespace we
{
  namespace type
  {
    class bytearray
    {
    public:
      void push_back (char c);

      bytearray();
      bytearray (const char* const, const std::size_t);
      bytearray (std::string const&);
      std::size_t copy (char* const buf, const std::size_t size) const;

      template<typename T>
      explicit bytearray (const T* const x)
        : _v ( static_cast<const char*> (static_cast<const void*> (x))
             , static_cast<const char*> (static_cast<const void*> (x)) + sizeof (*x)
             )
      {}
      template<typename T>
        explicit bytearray
          ( T const& x
          , typename boost::enable_if_c<not boost::is_pointer<T>::value>::type* = 0
          )
        : _v ( static_cast<const char*> (static_cast<const void*> (&x))
             , static_cast<const char*> (static_cast<const void*> (&x)) + sizeof (x)
             )
      {}
      template<typename T>
      std::size_t copy (T* const x) const
      {
        return copy
          (static_cast<char* const> (static_cast<void* const> (x)), sizeof (*x));
      }

      std::string to_string() const;

      friend std::ostream& operator<< (std::ostream&, const bytearray&);
      friend std::size_t hash_value (const bytearray&);
      friend bool operator== (const bytearray&, const bytearray&);
      friend bool operator< (const bytearray&, const bytearray&);

      template<typename T>
      bytearray& operator= (const T& other)
      {
        _v = std::vector<char>
          ( static_cast<const char*> (static_cast<const void*> (&other))
          , static_cast<const char*> (static_cast<const void*> (&other)) + sizeof (T)
          );

        return *this;
      }

      template<typename Archive>
        void serialize (Archive& ar, unsigned int)
      {
        ar & _v;
      }

    private:
      std::vector<char> _v;
    };
  }
}
