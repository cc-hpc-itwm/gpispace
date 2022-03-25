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
