// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
