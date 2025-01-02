// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/test/eureka/jobserver/EurekaGroup.hpp>
#include <we/test/eureka/jobserver/Task.hpp>
#include <we/test/eureka/jobserver/protocol.hpp>

#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>
#include <util-rpc/service_tcp_provider.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

namespace gspc
{
  namespace we
  {
    namespace test
    {
      namespace eureka
      {
        namespace jobserver
        {
          using TasksByEurekaGroup
            = std::unordered_map<EurekaGroup, std::unordered_set<Task>>
            ;

          struct JobServer
          {
            JobServer();

            std::string host() const;
            int port() const;

            struct Running{};
            struct Cancelled{};
            struct ExitedOrCancelled{};

            using State = std::variant<Running, Cancelled, ExitedOrCancelled>;

            void wait (std::size_t, EurekaGroup, State);
            void wait (std::size_t, TasksByEurekaGroup, State);

            TasksByEurekaGroup tasks (State);

          private:
            fhg::rpc::service_dispatcher _dispatcher;
            fhg::rpc::service_handler<protocol::running> _provide_running;
            fhg::rpc::service_handler<protocol::cancelled> _provide_cancelled;
            fhg::rpc::service_handler<protocol::exited_or_cancelled>
              _provide_exited_or_cancelled;
            fhg::util::scoped_boost_asio_io_service_with_threads _io_service;
            fhg::rpc::service_tcp_provider _provider;

            void running (Task, EurekaGroup);
            void cancelled (Task, EurekaGroup);
            void exited_or_cancelled (Task, EurekaGroup);

            struct Tasks
            {
              void add (Task, EurekaGroup);
              void wait (std::size_t);
              void wait (std::size_t, EurekaGroup);
              TasksByEurekaGroup tasks();

            private:
              std::mutex _guard;
              std::condition_variable _added;
              TasksByEurekaGroup _tasks_by_eureka_group;
              std::size_t _N = 0;
            };

            Tasks _running;
            Tasks _cancelled;
            Tasks _exited_or_cancelled;

            void wait (std::size_t, State);
          };
        }
      }
    }
  }
}
