// mirko.rahn@itwm.fraunhofer.de

#ifndef _BYTEARRAY_HPP
#define _BYTEARRAY_HPP

#include <algorithm>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <boost/functional/hash.hpp>

#include <iostream>
#include <sstream>

namespace bytearray
{
  class type
  {
  public:
    typedef std::vector<char> container_type;

    type () : _v (), _s(0) {}
    explicit type (const container_type & v) : _v (v), _s(0) {}

    type (const char * const buf, const std::size_t size) : _v(), _s(size)
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
    explicit type (const T * const x) : _v (), _s(sizeof(*x))
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

    const std::size_t & size () const { return _s; }
    const container_type & container () const { return _v; }

    friend std::ostream & operator << (std::ostream &, const type &);
    friend std::size_t hash_value (const type &);
    friend bool operator == (const type &, const type &);

  private:
    container_type _v;
    std::size_t _s;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(_v);
      ar & BOOST_SERIALIZATION_NVP(_s);
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

  template<typename T, typename Archive = boost::archive::binary_oarchive>
  class encoder
  {
  public:
    explicit encoder (const T & x) : _encoded (encode (x)) {}

    const type & operator * () const { return _encoded; }
    const type & bytearray () const { return _encoded; }

  private:
    const type _encoded;

    type encode (const T & x)
    {
      std::ostringstream oss;

      Archive oa (oss, boost::archive::no_header);

      oa << x;

      return type (oss.str().c_str(), oss.str().size());
    }
  };

  template<typename T, typename Archive = boost::archive::binary_iarchive>
  class decoder
  {
  public:
    explicit decoder (const type & ba) : _x (decode (ba)) {}

    const T & operator * () const { return _x; }
    const T & value () const { return _x; }

  private:
    const T _x;

    T decode (const type & ba)
    {
      std::istringstream iss(std::string (&ba.container()[0], ba.size()));

      Archive ia (iss, boost::archive::no_header);

      T x;

      ia >> x;

      return x;
    }
  };
}

#endif
