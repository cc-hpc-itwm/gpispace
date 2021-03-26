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

#include <boost/utility.hpp>

#include <condition_variable>
#include <mutex>

namespace fhg
{
  namespace util
  {
    namespace thread
    {
      template <typename T = void> class event;

      template <typename T>
        class event : boost::noncopyable
      {
        T _event;
        bool _signalled {false};
        std::mutex _mutex;
        std::condition_variable _condition;

      public:
        T wait()
        {
          std::unique_lock<std::mutex> lock (_mutex);

          _condition.wait (lock, [this] { return _signalled; });
          _signalled = false;

          return std::move (_event);
        }

        void notify (T u)
        {
          std::lock_guard<std::mutex> const _ (_mutex);

          _event = std::move (u);

          _signalled = true;
          _condition.notify_one();
        }
      };

      template<>
        class event<void> : boost::noncopyable
      {
        bool _signalled {false};
        std::mutex _mutex;
        std::condition_variable _condition;

      public:
        void wait()
        {
          std::unique_lock<std::mutex> lock (_mutex);

          _condition.wait (lock, [this] { return _signalled; });
          _signalled = false;
        }

        void notify()
        {
          std::lock_guard<std::mutex> const _ (_mutex);

          _signalled = true;
          _condition.notify_one();
        }
      };
    }
  }
}
