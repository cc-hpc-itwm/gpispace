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

#include <future>
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
        struct JobExecutor
        {
        private:
          Task const _task;
          std::string const _worker_name;
          std::promise<void> _released{};
          void release (Task, std::string worker_name);
          remote_function::Provider<protocol::release> const _provider
            {fhg::util::bind_this (this, &JobExecutor::release)};

        public:
          JobExecutor
            ( std::pair<std::string, unsigned short> job_server
            , Task
            , std::string worker_name
            );

          void wait_for_release() &&;
        };
      }
    }
  }
}
