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
