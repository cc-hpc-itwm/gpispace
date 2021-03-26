// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <we/type/bytearray.hpp>

#include <boost/functional/hash.hpp>

#include <iostream>

namespace we
{
  namespace type
  {
    void bytearray::push_back (char c)
    {
      _v.push_back (c);
    }

    bytearray::bytearray()
      : _v()
    {}
    bytearray::bytearray (const char* const buf, const std::size_t size)
      : _v (buf, buf + size)
    {}
    bytearray::bytearray (std::string const& s)
      : bytearray (s.data(), s.size())
    {}
    std::size_t bytearray::copy (char* const buf, const std::size_t size) const
    {
      const std::size_t s (std::min (_v.size(), size));

      std::copy (_v.begin(), _v.begin() + s, buf);

      return s;
    }
    std::string bytearray::to_string() const
    {
      return std::string (&_v[0], _v.size());
    }

    std::ostream& operator<< (std::ostream& s, const bytearray &t)
    {
      s << "y(";
      for (const char c : t._v)
      {
        s << " " << static_cast<unsigned long> (c);
      }
      return s << ")";
    }

    std::size_t hash_value (const bytearray &t)
    {
      return boost::hash<std::vector<char>>()(t._v);
    }

    bool operator== (const bytearray &x, const bytearray &y)
    {
      return x._v == y._v;
    }
    bool operator< (const bytearray &x, const bytearray &y)
    {
      return x._v < y._v;
    }
  }
}
