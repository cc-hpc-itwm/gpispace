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

#include <util-generic/cxx14/make_unique.hpp>

#include <boost/noncopyable.hpp>

#include <algorithm>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace fhg
{
  namespace thread
  {
    //! \note A set of threads which are cleaned up in dtor.
    class set : boost::noncopyable
    {
    public:
      void start (const std::function<void()> fun)
      {
        if (_stopped)
        {
          return;
        }

        const std::lock_guard<std::mutex> _ (_threads_mutex);
        _threads.emplace_back ( fhg::util::cxx14::make_unique<std::thread>
                                  (&set::thread_function, this, fun)
                              );
      }

      ~set()
      {
        std::vector<std::unique_ptr<std::thread>> threads;
        {
          std::lock_guard<std::mutex> const _ (_threads_mutex);
          _stopped = true;
          std::swap (threads, _threads);
        }

        for (std::unique_ptr<std::thread>& thread : threads)
        {
          if (thread->joinable())
          {
            thread->join();
          }
        }
      }

    private:
      void thread_function (const std::function<void()> fun)
      {
        fun();

        //! \note Remove from set to avoid leaking.
        {
          const std::lock_guard<std::mutex> _ (_threads_mutex);
          const std::vector<std::unique_ptr<std::thread>>::iterator it
            ( std::find_if ( _threads.begin(), _threads.end()
                           , [] (std::unique_ptr<std::thread> const& t)
                           {
                             return t->get_id() == std::this_thread::get_id();
                           }
                           )
            );
          if (it != _threads.end())
          {
            _threads.erase (it);
          }
        }
      }

      std::mutex _threads_mutex;
      std::vector<std::unique_ptr<std::thread>> _threads;
      bool _stopped = false;
    };
  }
}
