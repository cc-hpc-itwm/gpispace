// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <test/parallel_reduce/module_call/Task.hpp>
#include <test/parallel_reduce/module_call/protocol.hpp>
#include <test/parallel_reduce/module_call/remote_function/Provider.hpp>

#include <util-generic/this_bound_mem_fn.hpp>

#include <condition_variable>
#include <list>
#include <mutex>
#include <string>
#include <utility>

namespace gspc
{
  namespace test
  {
    namespace parallel_reduce
    {
      namespace module_call
      {
        struct JobServer
        {
        private:
          unsigned long _number_of_elements;
          unsigned long _number_of_workers;

          struct RunningTask
          {
            Task task;
            std::string worker_name;
            std::pair<std::string, unsigned short> address;
          };

          std::mutex _guard_running_tasks{};
          std::condition_variable _task_finished{};
          std::list<RunningTask> _running_tasks{};

          decltype (_running_tasks.size()) _max_parallel_running_tasks {0};

          void running
            ( Task
            , std::string worker_name
            , std::pair<std::string, unsigned short> release_address
            );

          remote_function::Provider<protocol::running> _provider
            {fhg::util::bind_this (this, &JobServer::running)};

        public:
          JobServer
            ( decltype (_number_of_elements)
            , decltype (_number_of_workers)
            );

          auto address() const -> decltype (_provider.address());

          auto wait_and_return_max_parallel_running_tasks() &&
            -> decltype (_max_parallel_running_tasks);
        };
      }
    }
  }
}
