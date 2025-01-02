// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
