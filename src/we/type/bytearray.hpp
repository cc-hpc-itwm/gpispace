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

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <boost/serialization/vector.hpp>

#include <algorithm>
#include <iosfwd>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace we
{
  namespace type
  {
    class GSPC_DLLEXPORT bytearray
    {
    public:
      void push_back (char c);

      bytearray();
      bytearray (std::vector<char> v) : _v (std::move (v)) {}
      bytearray (const char* const, std::size_t);
      bytearray (std::string const&);
      std::size_t copy (char* const buf, std::size_t size) const;

      template<typename T>
      explicit bytearray (const T* const x)
        : _v ( static_cast<const char*> (static_cast<const void*> (x))
             , static_cast<const char*> (static_cast<const void*> (x)) + sizeof (*x)
             )
      {}
      template<typename T>
        explicit bytearray
          ( T const& x
          , typename std::enable_if<not std::is_pointer<T>::value>::type* = nullptr
          )
        : _v ( static_cast<const char*> (static_cast<const void*> (&x))
             , static_cast<const char*> (static_cast<const void*> (&x)) + sizeof (x)
             )
      {}
      template<typename T>
      std::size_t copy (T* const x) const
      {
        return copy
          (static_cast<char*> (static_cast<void*> (x)), sizeof (*x));
      }

      std::string to_string() const;
      std::vector<char> const& v() const { return _v; }

      GSPC_DLLEXPORT
        friend std::ostream& operator<< (std::ostream&, bytearray const&);
      GSPC_DLLEXPORT
        friend std::size_t hash_value (bytearray const&);
      GSPC_DLLEXPORT
        friend bool operator== (bytearray const&, bytearray const&);
      GSPC_DLLEXPORT
        friend bool operator< (bytearray const&, bytearray const&);

      template<typename T>
      bytearray& operator= (T const& other)
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
