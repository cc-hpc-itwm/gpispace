// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
