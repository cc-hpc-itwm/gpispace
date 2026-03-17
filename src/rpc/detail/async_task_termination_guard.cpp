#include <gspc/rpc/detail/async_task_termination_guard.hpp>

#include <algorithm>
#include <utility>



    namespace gspc::rpc::detail
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
