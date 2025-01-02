// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/parallel_reduce/module_call/JobServer.hpp>

#include <test/parallel_reduce/module_call/protocol.hpp>
#include <test/parallel_reduce/module_call/remote_function/Client.hpp>

#include <algorithm>
#include <iterator>

namespace gspc
{
  namespace test
  {
    namespace parallel_reduce
    {
      namespace module_call
      {
        void JobServer::running
          ( Task task
          , std::string worker_name
          , std::pair<std::string, unsigned short> release_address
          )
        {
          std::lock_guard<std::mutex> const lock {_guard_running_tasks};

          _running_tasks.emplace_back
            (RunningTask {task, worker_name, release_address});

          _max_parallel_running_tasks
            = std::max (_max_parallel_running_tasks, _running_tasks.size())
            ;

          _number_of_elements -= 2;

          auto const some_workers_are_free
            ( [&]
              {
                return _running_tasks.size() < _number_of_workers;
              }
            );
          auto const tasks_must_be_finished_to_not_stall
            ( [&]
              {
                return _number_of_elements < 2 && !_running_tasks.empty();
              }
            );

          while (  !some_workers_are_free()
                || tasks_must_be_finished_to_not_stall()
                )
          {
            auto const running_task (std::begin (_running_tasks));

            remote_function::Client<protocol::release> {running_task->address}
              ( running_task->task
              , running_task->worker_name
              );

            _number_of_elements += 1;

            _running_tasks.erase (running_task);

            _task_finished.notify_one();
          }
        }

        JobServer::JobServer
          ( decltype (_number_of_elements) number_of_elements
          , decltype (_number_of_workers) number_of_workers
          )
            : _number_of_elements (number_of_elements)
            , _number_of_workers (number_of_workers)
        {}

        auto JobServer::address() const -> decltype (_provider.address())
        {
          return _provider.address();
        }

        auto JobServer::wait_and_return_max_parallel_running_tasks() &&
          -> decltype (_max_parallel_running_tasks)
        {
          std::unique_lock<std::mutex> lock {_guard_running_tasks};

          _task_finished.wait
            ( lock
            , [&]
              {
                return _running_tasks.empty() && _number_of_elements == 1;
              }
            );

          return _max_parallel_running_tasks;
        }
      }
    }
  }
}
