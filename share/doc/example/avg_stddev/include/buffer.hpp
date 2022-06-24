// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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
    //! \todo use std::span
    template<typename T>
    struct type
    {
    private:
      T* _begin {nullptr};
      std::size_t _count {0};
      std::size_t _size {0};

    public:
      type() = default;
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
