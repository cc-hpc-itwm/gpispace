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
      //! \brief: Do not use this class in new code. Consider to use
      //! `std::promise` or `fhg::util::threadsafe_queue` instead.
      //!
      //! The interface is unsafe:
      //!
      //! It does not prevent clients from overwriting notified
      //! values, e.g. multiple notify without wait are possible.
      //!
      //! It does not prevent clients to re-use the same event, which
      //! is dangerous when there is no protection from overwrite.
      //!
      //! It is not sticky, e.g. the client timing is relevant to not
      //! miss a notification.
      //!
      //! Suppose the situation:
      //!
      //! Thread  1           2           3       4
      //! Action  notify (1)  notify (2)  wait()  wait()
      //!
      //! Depending on the timing there might be multiple outcomes, e.g.
      //! both waiting threads get a result or one waiting thread gets
      //! one of the results and the other waiting thread blocks.
      //!
      //! EXAMPLE for case 1
      //!         notify (1)
      //!                                 wait()
      //!                     notify (2)
      //!                                         wait()
      //! => 3 gets 1, 4 gets 2
      //!
      //! EXAMPLE for case 2
      //!         notify (1)  notify (2)
      //!                                 wait()
      //!                                         wait()
      //! => 3 gets 1 or 2, 4 blocks
      //!
      template <typename T>
        class UNSAFE_event : ::boost::noncopyable
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
    }
  }
}
