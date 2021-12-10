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

#include <cstddef>

namespace fhg
{
  namespace buffer
  {
    template<typename T>
    struct type
    {
    private:
      T* _begin;
      std::size_t _count;
      std::size_t _size;

    public:
      type () : _begin (nullptr), _count (0), _size (0) {}
      type (const type& other)
        : _begin (other._begin)
        , _count (other._count)
        , _size (other._size)
      {}
      type& operator = (const type& other)
      {
        _begin = other._begin;
        _count = other._count;
        _size = other._size;
        return *this;
      }
      explicit type (T* begin, const std::size_t size)
        : _begin (begin)
        , _count (size)
        , _size (size)
      {
      }
      T* begin () { return _begin; }
      T* end () { return _begin + _count; }
      const std::size_t& count () const { return _count; }
      std::size_t& count () { return _count; }
      const std::size_t& size () const { return _size; }
    };
  }
}
