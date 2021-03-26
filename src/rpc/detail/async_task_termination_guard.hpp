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
#include <cstddef>
#include <memory>
#include <mutex>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      //! Counter with ability to wait for reaching zero, to use for
      //! keeping track of asynchronous operations in threads that
      //! can't be joined, while being free of destruction race.
      //! \note Very close to util-generic's latch, but with added
      //! increment-without-wait and specific for the semantics
      //! required here.
      struct async_task_termination_guard
      {
        //! Call in the destructor of the object owning the
        //! tasks.
        //! When this function returns it is guaranteed that the task
        //! is not going to access anything unsafe anymore and there
        //! are no further observable effects from that task.
        //! \note Not calling this function defeats the purpose. It is
        //! not the destructor though as usually additional cleanup
        //! needs to happen after the tasks are done.
        void wait_for_termination();

        //! `boost::asio::spawn()` a \a task on \a executor, and keep
        //! track of that.
        template<typename Executor, typename Task>
          void spawn_task (Executor& executor, Task&& task);

        async_task_termination_guard();
        async_task_termination_guard
          (async_task_termination_guard const&) = delete;
        async_task_termination_guard (async_task_termination_guard&&) = delete;
        async_task_termination_guard& operator=
          (async_task_termination_guard const&) = delete;
        async_task_termination_guard& operator=
          (async_task_termination_guard&&) = delete;
        ~async_task_termination_guard() = default;

      private:
        struct state
        {
          std::mutex _guard;
          std::condition_variable _counted_down;
          std::size_t _count = 0;
        };

        struct mark_task_scope
        {
          mark_task_scope() = delete;
          mark_task_scope (mark_task_scope const&) = delete;
          mark_task_scope (mark_task_scope&) = delete;
          mark_task_scope& operator= (mark_task_scope const&) = delete;
          mark_task_scope& operator= (mark_task_scope&) = delete;
          ~mark_task_scope();

          mark_task_scope (std::shared_ptr<state>);
        private:
          std::shared_ptr<state> _state;
        };

      private:
        //! Shared to avoid destruction race: Every task has a copy of
        //! the shared state, and as `~mask_task_scope` is guaranteed
        //! to be last, it is fine to only protect this state.
        std::shared_ptr<state> _state;
      };
    }
  }
}

#include <rpc/detail/async_task_termination_guard.ipp>
