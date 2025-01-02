// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>

namespace fhg
{
  namespace thread
  {
    template < typename T
             , template < typename
                        , typename
                        > class Container = std::deque
             , typename Allocator = std::allocator<T>
             >
    class queue
    {
    public:
      T get ()
      {
        std::unique_lock<std::mutex> lock (_guard_container);

        _container_not_empty.wait (lock, [&] { return !_container.empty(); });

        T t (_container.front());

        _container.pop_front();

        return t;
      }

      void put (T const & t)
      {
        std::lock_guard<std::mutex> const lock (_guard_container);

        _container.push_back (t);

        _container_not_empty.notify_one();
      }

    private:
      Container<T, Allocator> _container;
      std::condition_variable _container_not_empty;
      std::mutex _guard_container;
    };
  }
}
