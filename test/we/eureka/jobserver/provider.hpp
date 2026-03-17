// Copyright (C) 2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <test/we/eureka/jobserver/EurekaGroup.hpp>
#include <test/we/eureka/jobserver/Task.hpp>
#include <test/we/eureka/jobserver/protocol.hpp>

#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>

#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>





        namespace gspc::we::test::eureka::jobserver
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
            gspc::rpc::service_dispatcher _dispatcher;
            gspc::rpc::service_handler<protocol::running> _provide_running;
            gspc::rpc::service_handler<protocol::cancelled> _provide_cancelled;
            gspc::rpc::service_handler<protocol::exited_or_cancelled>
              _provide_exited_or_cancelled;
            gspc::util::scoped_boost_asio_io_service_with_threads _io_service;
            gspc::rpc::service_tcp_provider _provider;

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
