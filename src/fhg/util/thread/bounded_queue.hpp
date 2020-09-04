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

#include <boost/utility.hpp>

#include <condition_variable>
#include <list>
#include <mutex>

namespace fhg
{
  namespace thread
  {
    template <typename T>
      class bounded_queue : public boost::noncopyable
    {
    public:
      bounded_queue (std::size_t capacity)
        :_capacity(capacity)
      {}

      std::pair<T, bool> get()
      {
        std::unique_lock<std::mutex> lock (_mtx);
        _cond_non_empty.wait (lock, [this] { return !_container.empty(); });

        T t (_container.front());
        _container.pop_front();

        return std::make_pair (t, _container.size() == _capacity-1);
      }

      template<class... Args>
      bool try_put (Args&&... args)
      {
        std::unique_lock<std::mutex> const _ (_mtx);

        if (_container.size() < _capacity)
        {
          _container.emplace_back (std::forward<Args> (args)...);
          _cond_non_empty.notify_one();
          return true;
        }

        return false;
      }

      std::size_t capacity() {return _capacity;}

    private:
      std::mutex _mtx;
      std::condition_variable _cond_non_empty;

      std::list<T> _container;
      std::size_t _capacity;
    };
  }
}
