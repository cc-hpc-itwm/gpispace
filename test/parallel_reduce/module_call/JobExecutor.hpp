// Copyright (C) 2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <test/parallel_reduce/module_call/Task.hpp>
#include <test/parallel_reduce/module_call/protocol.hpp>
#include <test/parallel_reduce/module_call/remote_function/Provider.hpp>

#include <gspc/util/this_bound_mem_fn.hpp>

#include <future>
#include <string>
#include <utility>




      namespace gspc::test::parallel_reduce::module_call
      {
        struct JobExecutor
        {
        private:
          Task const _task;
          std::string const _worker_name;
          std::promise<void> _released{};
          void release (Task, std::string worker_name);
          remote_function::Provider<protocol::release> const _provider
            {gspc::util::bind_this (this, &JobExecutor::release)};

        public:
          JobExecutor
            ( std::pair<std::string, unsigned short> job_server
            , Task
            , std::string worker_name
            );

          void wait_for_release() &&;
        };
      }
