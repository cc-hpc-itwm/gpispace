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

#include <util-rpc/detail/async_task_termination_guard.hpp>

#include <algorithm>
#include <utility>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      async_task_termination_guard::async_task_termination_guard()
        : _state (std::make_shared<state>())
      {}

      void async_task_termination_guard::wait_for_termination()
      {
        std::unique_lock<std::mutex> lock (_state->_guard);
        _state->_counted_down.wait (lock, [&] { return _state->_count == 0; });
      }

      async_task_termination_guard::mark_task_scope::mark_task_scope (std::shared_ptr<state> state)
        : _state (std::move (state))
      {}
      async_task_termination_guard::mark_task_scope::~mark_task_scope()
      {
        std::lock_guard<std::mutex> const lock (_state->_guard);
        _state->_count--;
        _state->_counted_down.notify_all();
      }
    }
  }
}
