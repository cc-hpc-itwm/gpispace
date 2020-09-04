// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace error
    {
      inline count_down_for_zero::count_down_for_zero()
        : std::logic_error ("latch: count_down for count == 0")
      {}
    }

    inline latch::latch (std::size_t count)
      : _guard()
      , _counted_down()
      , _count (count)
    {}

    inline void latch::wait()
    {
      std::unique_lock<std::mutex> lock (_guard);
      _counted_down.wait (lock, [this] { return _count == 0; });
    }

    inline void latch::wait_and_reset (std::size_t count)
    {
      std::unique_lock<std::mutex> lock (_guard);
      _counted_down.wait (lock, [this] { return _count == 0; });
      _count = count;
    }

    inline void latch::count_down()
    {
      std::lock_guard<std::mutex> const _ (_guard);
      if (_count == 0)
      {
        throw error::count_down_for_zero();
      }
      --_count;
      _counted_down.notify_all();
    }
  }
}
