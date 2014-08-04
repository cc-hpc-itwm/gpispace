// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_BYTEARRAY_HPP
#define _WE_TYPE_BYTEARRAY_HPP

#include <boost/type_traits/is_pointer.hpp>
#include <boost/utility/enable_if.hpp>

#include <algorithm>
#include <vector>

#include <iosfwd>
#include <sstream>

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
      std::size_t copy (char* const buf, const std::size_t size) const;

      template<typename T>
      explicit bytearray (const T* const x)
        : _v()
      {
        std::copy ((char *)x, (char *)x + sizeof (*x), std::back_inserter(_v));
      }
      template<typename T>
        explicit bytearray
          ( T const& x
          , typename boost::enable_if_c<not boost::is_pointer<T>::value>::type* = 0
          )
        : bytearray (&x)
      {}
      template<typename T>
      std::size_t copy (T* const x) const
      {
        const std::size_t s (std::min (_v.size(), sizeof (*x)));

        std::copy (_v.begin(), _v.begin() + s, (char *)x);

        return s;
      }

      std::string to_string() const;

      friend std::ostream& operator<< (std::ostream&, const bytearray&);
      friend std::size_t hash_value (const bytearray&);
      friend bool operator== (const bytearray&, const bytearray&);
      friend bool operator< (const bytearray&, const bytearray&);

      template<typename T>
      bytearray& operator= (const T& other)
      {
        _v.clear();

        std::copy ( (char*)&other
                  , (char*)&other + sizeof (T)
                  , std::back_inserter(_v)
                  );

        return *this;
      }

    private:
      std::vector<char> _v;
    };
  }
}

#endif
