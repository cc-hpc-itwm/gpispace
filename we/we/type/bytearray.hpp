// mirko.rahn@itwm.fraunhofer.de

#ifndef _BYTEARRAY_HPP
#define _BYTEARRAY_HPP

#include <algorithm>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <iosfwd>
#include <sstream>

namespace bytearray
{
  class type
  {
  public:
    void push_back (char c);

    type();
    type (const type&);
    type (const char* const, const std::size_t);
    std::size_t copy (char* const buf, const std::size_t size) const;

    template<typename T>
    explicit type (const T* const x)
      : _v()
    {
      std::copy ((char *)x, (char *)x + sizeof (*x), std::back_inserter(_v));
    }
    template<typename T>
    std::size_t copy (T* const x) const
    {
      const std::size_t s (std::min (_v.size(), sizeof (*x)));

      std::copy (_v.begin(), _v.begin() + s, (char *)x);

      return s;
    }
    template<typename T>
    operator T() const
    {
      T x;

      copy (&x);

      return x;
    }

    std::size_t size() const;
    const std::vector<char>& container() const;

    friend std::ostream& operator<< (std::ostream&, const type&);
    friend std::size_t hash_value (const type&);
    friend bool operator== (const type&, const type&);

    type& operator= (const type& other);

    template<typename T>
    type& operator= (const T& other)
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

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(_v);
    }
  };

  template<typename T, typename Archive = boost::archive::binary_oarchive>
  class encoder
  {
  public:
    const type& bytearray() const
    {
      return _encoded;
    }

    explicit encoder (const T& x)
      : _encoded()
    {
      std::ostringstream oss;

      Archive oa (oss, boost::archive::no_header);

      oa << x;

      _encoded = type (oss.str().c_str(), oss.str().size());
    }

  private:
    type _encoded;
  };

  template<typename T, typename Archive = boost::archive::binary_iarchive>
  class decoder
  {
  public:
    const T& value() const
    {
      return _x;
    }
    T& value()
    {
      return _x;
    }

    explicit decoder (const type& ba)
      : _x()
    {
      std::istringstream iss (std::string (&ba.container()[0], ba.size()));

      Archive ia (iss, boost::archive::no_header);

      ia >> _x;
    }

  private:
    T _x;
  };
}

#endif
