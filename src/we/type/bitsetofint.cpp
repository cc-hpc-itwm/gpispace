// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/bitsetofint.hpp>

#include <algorithm>
#include <stdexcept>

#include <boost/functional/hash.hpp>

#include <iomanip>
#include <iostream>
#include <sstream>

#include <stdint.h>

namespace bitsetofint
{
  type::type (std::size_t n)
    : _container (n)
  {}

  void type::push_back (uint64_t v)
  {
    _container.push_back (v);
  }

  type& type::ins (unsigned long const& x)
  {
    if ((x >> 6) >= _container.size())
    {
      _container.resize((x >> 6) + 1, 0);
    }
    _container[x >> 6] |= (1UL << (x & 63));

    return *this;
  }
  type& type::del (unsigned long const& x)
  {
    if ((x >> 6) < _container.size())
    {
      _container[(x >> 6)] &= ~(1UL << (x & 63));

      while (_container.size() && !_container.back())
      {
        _container.pop_back();
      }
    }

    return *this;
  }
  bool type::is_element (unsigned long const& x) const
  {
    return ((x >> 6) < _container.size())
      && ((_container[(x >> 6)] & (1UL << (x & 63))) != 0);
  }
  std::size_t type::count() const
  {
    std::size_t cnt (0);

    for (uint64_t block : _container)
    {
      // http://en.wikipedia.org/wiki/Hamming_weight

      const uint64_t m1  (0x5555555555555555);
      const uint64_t m2  (0x3333333333333333);
      const uint64_t m4  (0x0f0f0f0f0f0f0f0f);
      const uint64_t h01 (0x0101010101010101);

      block -= (block >> 1) & m1;
      block = (block & m2) + ((block >> 2) & m2);
      block = (block + (block >> 4)) & m4;
      cnt += (block * h01) >> 56;
    }

    return cnt;
  }

  void type::list (std::ostream& s) const
  {
    list ([&s] (unsigned long const& x) { s << x << std::endl; });
  }
  void type::list (std::function<void (unsigned long const&)> const& f) const
  {
    unsigned long x (0);

    for (uint64_t block : _container)
    {
      for (size_t bit (0); bit < 64; ++x, ++bit, block >>= 1)
      {
        if (block & 0x1)
        {
          f (x);
        }
      }
    }
  }

  std::set<unsigned long> type::elements() const
  {
    std::set<unsigned long> s;

    list ([&s] (unsigned long const& x) { s.insert (x); });

    return s;
  }

  type operator| (type const& lhs, type const& rhs)
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
  type operator& (type const& lhs, type const& rhs)
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

  type operator^ (type const& lhs, type const& rhs)
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

  std::ostream& operator<< (std::ostream& s, type const& t)
  {
    s << "{";
    for (uint64_t v : t._container)
    {
      s << " " << v;
    }
    return s << "}";
  }
  std::string to_hex (type const& t)
  {
    std::ostringstream oss;

    oss << "0x/";

    for (uint64_t v : t._container)
    {
      oss.flags (std::ios::hex);
      oss.width (16);
      oss.fill ('0');
      oss << v << '/';
    }

    return oss.str();
  }

  ::boost::optional<type> from_hex ( std::string::const_iterator& pos
                                 , std::string::const_iterator const& end
                                 )
  {
    if (  (pos + 0) != end && *(pos + 0) == '0'
       && (pos + 1) != end && *(pos + 1) == 'x'
       )
    {
      std::advance (pos, 2);

      type bs;

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
            return std::move (bs);
          }

          bs.push_back (value);

          std::advance (pos, 16);
        }
      }

      return std::move (bs);
    }

    return ::boost::none;
  }

  type from_hex (std::string const& s)
  {
    std::string::const_iterator pos (s.begin());
    std::string::const_iterator const& end (s.end());

    ::boost::optional<type> mtype (from_hex (pos, end));

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

  std::size_t hash_value (type const& t)
  {
    ::boost::hash<std::vector<uint64_t>> h;

    return h (t._container);
  }

  bool operator== (type const& x, type const& y)
  {
    auto pos_x (x._container.begin());
    const std::vector<uint64_t>::const_iterator& end_x (x._container.end());
    auto pos_y (y._container.begin());
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

  bool operator< (type const& x, type const& y)
  {
    return x._container < y._container;
  }
}
