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
      typedef std::mutex mutex_type;
      typedef std::unique_lock<mutex_type> lock_type;
      typedef std::condition_variable condition_type;

      typedef T value_type;
      typedef Container<T, Allocator> container_type;
      typedef typename container_type::size_type size_type;

      T get ()
      {
        lock_type lock (_mtx);

        while (_container.empty())
          {
            _cond.wait(lock);
          }

        T t (_container.front());

        _container.pop_front();

        return t;
      }

      void put (T const & t)
      {
        lock_type lock (_mtx);

        _container.push_back (t);

        _cond.notify_one();
      }

    private:
      mutable mutex_type _mtx;
      mutable condition_type _cond;

      container_type _container;
    };
  }
}
