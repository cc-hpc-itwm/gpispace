// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <boost/asio/spawn.hpp>

#include <utility>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      template<typename Executor, typename Task>
        void async_task_termination_guard::spawn_task
          (Executor& executor, Task&& task)
      {
        auto task_state (_state);
        {
          std::lock_guard<std::mutex> const lock (task_state->_guard);
          task_state->_count++;
        }
        // \todo C++14: move/forward task and task_state. Then, also
        // move ++ into ctor and move scope guard into functor.
        ::boost::asio::spawn
          ( executor
          , [task, task_state] (::boost::asio::yield_context yield)
            {
              mark_task_scope const scope_marker {std::move (task_state)};
              task (std::move (yield));
            }
          );
      }
    }
  }
}
