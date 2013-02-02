// mirko.rahn@itwm.fraunhofer.de

#include <we/type/bytearray.hpp>

#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>

#include <iostream>

namespace bytearray
{
  void type::push_back (char c)
  {
    _v.push_back (c);
  }

  type::type()
    : _v()
  {}
  type::type (const type& other)
    : _v (other._v)
  {}
  type::type (const char* const buf, const std::size_t size)
    : _v()
  {
    std::copy (buf, buf + size, std::back_inserter (_v));
  }
  std::size_t type::copy (char* const buf, const std::size_t size) const
  {
    const std::size_t s (std::min (_v.size(), size));

    std::copy (_v.begin(), _v.begin() + s, buf);

    return s;
  }
  std::size_t type::size() const
  {
    return _v.size();
  }
  const std::vector<char>& type::container() const
  {
    return _v;
  }
  type& type::operator= (const type& other)
  {
    if (this != &other)
    {
      _v = other._v;
    }

    return *this;
  }

  std::ostream& operator<< (std::ostream& s, const type& t)
  {
    s << "y(";
    BOOST_FOREACH (const char c, t._v)
      {
        s << " " << int (c);
      }
    return s << ")";
  }

  std::size_t hash_value (const type& t)
  {
    return boost::hash<std::vector<char> >()(t._v);
  }

  bool operator== (const type& x, const type& y)
  {
    return x._v == y._v;
  }
}
