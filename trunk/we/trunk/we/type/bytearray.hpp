// mirko.rahn@itwm.fraunhofer.de

#ifndef _BYTEARRAY_HPP
#define _BYTEARRAY_HPP

#include <algorithm>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

namespace bytearray
{
  class type
  {
  public:
    type () : _v () {}

    type (const char * buf, const std::size_t size) : _v ()
    {
      std::copy (buf, buf + size, std::back_inserter (_v));
    }
    std::size_t copy (char * buf, const std::size_t size)
    {
      const std::size_t s (std::min (_v.size(), size));

      std::copy (_v.begin(), _v.begin() + s, buf);

      return s;
    }

    template<typename T>
    explicit type (T * x) : _v ()
    {
      std::copy ((char *)x, (char *)x + sizeof (*x), std::back_inserter(_v));
    }
    template<typename T>
    std::size_t copy (T * x)
    {
      const std::size_t s (std::min (_v.size(), sizeof (*x)));

      std::copy (_v.begin(), _v.begin() + s, (char *)x);

      return s;
    }

  private:
    std::vector<char> _v;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(_v);
    }
  };
}

#endif
