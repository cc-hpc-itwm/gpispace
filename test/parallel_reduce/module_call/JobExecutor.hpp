// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
