// mirko.rahn@itwm.fraunhofer.de

#include <we/type/bitsetofint.hpp>

#include <algorithm>
#include <stdexcept>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/functional/hash.hpp>

#include <boost/optional.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>

#include <stdint.h>

namespace bitsetofint
{
  namespace
  {
    inline std::size_t the_container (const unsigned long& x)
    {
      return (x >> 6);
    }
    inline std::size_t the_slot (const unsigned long& x)
    {
      return (x & 63);
    }
  }

  type::type (const std::vector<uint64_t>& c)
    : _container (c)
  {}
  type::type (const std::size_t n)
    : _container (n)
  {}

  type& type::ins (const unsigned long& x)
  {
    if (the_container(x) >= _container.size())
    {
      _container.resize(the_container(x) + 1, 0);
    }
    _container[the_container(x)] |= (1UL << the_slot(x));

    return *this;
  }
  type& type::del (const unsigned long& x)
  {
    if (the_container(x) < _container.size())
    {
      _container[the_container(x)] &= ~(1UL << the_slot(x));
    }

    return *this;
  }
  bool type::is_element (const unsigned long& x) const
  {
    return (the_container(x) < _container.size())
      && ((_container[the_container(x)] & (1UL << the_slot(x))) != 0);
  }
  std::size_t type::count() const
  {
    std::size_t cnt (0);

    BOOST_FOREACH (const uint64_t block, _container)
    {
      for (size_t bit (0); bit < 64; ++bit)
      {
        cnt += (block >> bit) & 0x1;
      }
    }

    return cnt;
  }
  void type::list (std::ostream& s) const
  {
    size_t x (0);

    BOOST_FOREACH (uint64_t block, _container)
    {
      for (size_t bit (0); bit < 64; ++x, ++bit, block >>= 1)
      {
        if (block & 0x1)
        {
          s << x << std::endl;
        }
      }
    }
  }

  type operator| (const type& lhs, const type& rhs)
  {
    type result (std::max (lhs._container.size(), rhs._container.size()));

    for (size_t i (0); i < result._container.size(); ++i)
    {
      result._container [i] =
        (
         (i < lhs._container.size() ? lhs._container[i] : 0)
         |
         (i < rhs._container.size() ? rhs._container[i] : 0)
        );
    }

    return result;
  }
  type operator& (const type& lhs, const type& rhs)
  {
    type result (std::max (lhs._container.size(), rhs._container.size()));

    for (size_t i (0); i < result._container.size(); ++i)
    {
      result._container [i] =
        (
         (i < lhs._container.size() ? lhs._container[i] : 0)
         &
         (i < rhs._container.size() ? rhs._container[i] : 0)
        );
    }

    return result;
  }

  type operator^ (const type& lhs, const type& rhs)
  {
    type result (std::max (lhs._container.size(), rhs._container.size()));

    for (size_t i (0); i < result._container.size(); ++i)
    {
      result._container [i] =
        (
         (i < lhs._container.size() ? lhs._container[i] : 0)
         ^
         (i < rhs._container.size() ? rhs._container[i] : 0)
        );
    }

    return result;
  }

  std::ostream& operator<< (std::ostream& s, const type& t)
  {
    s << "{";
    BOOST_FOREACH (const uint64_t v, t._container)
    {
      s << " " << v;
    }
    return s << "}";
  }
  std::string to_hex (const type& t)
  {
    std::ostringstream oss;

    oss << "0x/";

    BOOST_FOREACH (const uint64_t v,  t._container)
    {
      oss.flags (std::ios::hex);
      oss.width (16);
      oss.fill ('0');
      oss << v << '/';
    }

    return oss.str();
  }

  boost::optional<type> from_hex ( std::string::const_iterator& pos
                                 , const std::string::const_iterator& end
                                 )
  {
    if (  (pos + 0) != end && *(pos + 0) == '0'
       && (pos + 1) != end && *(pos + 1) == 'x'
       )
    {
      std::advance (pos, 2);

      std::vector<uint64_t> container;

      while (pos != end && *pos == '/')
      {
        std::advance (pos, 1);

        if (std::distance (pos, end) >= 16)
        {
          uint64_t value (0);

          std::istringstream iss (std::string (pos, pos + 16));

          iss.flags (std::ios::hex);
          iss.width (16);
          iss.fill ('0');

          iss >> value;

          if (iss.fail() && !iss.eof())
          {
            return type (container);
          }

          container.push_back (value);

          std::advance (pos, 16);
        }
      }

      return type (container);
    }

    return boost::none;
  }

  type from_hex (const std::string& s)
  {
    std::string::const_iterator pos (s.begin());
    const std::string::const_iterator& end (s.end());

    boost::optional<type> mtype (from_hex (pos, end));

    if (!mtype)
    {
      throw std::runtime_error ("bitsetofint::from_hex: missing prefix 0x");
    }

    if (pos != end)
    {
      throw std::runtime_error
        ("bitsetofint::from_hex invalid argument: \"" + s + "\""
        + ", rest after parsing: " + std::string (pos, end)
        );
    }

    return *mtype;
  }

  std::size_t hash_value (const type& t)
  {
    boost::hash<std::vector<uint64_t> > h;

    return h(t._container);
  }

  bool operator== (const type& x, const type& y)
  {
    std::vector<uint64_t>::const_iterator pos_x (x._container.begin());
    const std::vector<uint64_t>::const_iterator& end_x (x._container.end());
    std::vector<uint64_t>::const_iterator pos_y (y._container.begin());
    const std::vector<uint64_t>::const_iterator& end_y (y._container.end());

    while (pos_x != end_x && pos_y != end_y)
    {
      if (*pos_x != *pos_y)
      {
        return false;
      }

      ++pos_x;
      ++pos_y;
    }

    while (pos_x != end_x)
    {
      if (*pos_x != 0)
      {
        return false;
      }

      ++pos_x;
    }

    while (pos_y != end_y)
    {
      if (*pos_y != 0)
      {
        return false;
      }

      ++pos_y;
    }

    return true;
  }
}
