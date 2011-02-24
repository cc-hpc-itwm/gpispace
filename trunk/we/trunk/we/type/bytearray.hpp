// mirko.rahn@itwm.fraunhofer.de

#ifndef _BYTEARRAY_HPP
#define _BYTEARRAY_HPP

#include <algorithm>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/functional/hash.hpp>

#include <iostream>

namespace bytearray
{
  class type
  {
  public:
    typedef std::vector<char> container_type;

    type () : _v () {}
    explicit type (const container_type & v) : _v (v) {}

    type (const char * const buf, const std::size_t size) : _v ()
    {
      std::copy (buf, buf + size, std::back_inserter (_v));
    }
    std::size_t copy (char * const buf, const std::size_t size) const
    {
      const std::size_t s (std::min (_v.size(), size));

      std::copy (_v.begin(), _v.begin() + s, buf);

      return s;
    }

    template<typename T>
    explicit type (const T * const x) : _v ()
    {
      std::copy ((char *)x, (char *)x + sizeof (*x), std::back_inserter(_v));
    }
    template<typename T>
    std::size_t copy (T * const x) const
    {
      const std::size_t s (std::min (_v.size(), sizeof (*x)));

      std::copy (_v.begin(), _v.begin() + s, (char *)x);

      return s;
    }

    friend std::ostream & operator << (std::ostream &, const type &);
    friend std::size_t hash_value (const type &);
    friend bool operator == (const type &, const type &);

  private:
    container_type _v;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(_v);
    }
  };

  inline std::ostream & operator << (std::ostream & s, const type & t)
  {
    s << "y(";
    for ( type::container_type::const_iterator it (t._v.begin())
        ; it != t._v.end()
        ; ++it
        )
      {
        s << " " << int (*it);
      }
    return s << ")";
  }

  inline std::size_t hash_value (const type & t)
  {
    boost::hash<type::container_type> h;

    return h(t._v);
  }

  inline bool operator == (const type & x, const type & y)
  {
    return x._v == y._v;
  }
}

#endif
