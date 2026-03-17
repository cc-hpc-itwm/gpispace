// Copyright (C) 2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/we/eureka/jobserver/provider.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/functor_visitor.hpp>
#include <gspc/testing/printer/unordered_map.hpp>
#include <gspc/testing/printer/unordered_set.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/util/this_bound_mem_fn.hpp>





        namespace gspc::we::test::eureka::jobserver
        {
          JobServer::JobServer()
            : _dispatcher()
            , _provide_running
              ( _dispatcher
              , gspc::util::bind_this (this, &JobServer::running)
              )
            , _provide_cancelled
              ( _dispatcher
              , gspc::util::bind_this (this, &JobServer::cancelled)
              )
            , _provide_exited_or_cancelled
              ( _dispatcher
              , gspc::util::bind_this (this, &JobServer::exited_or_cancelled)
              )
            , _io_service (1)
            , _provider (_io_service, _dispatcher)
          {}

          std::string JobServer::host() const
          {
            return gspc::util::connectable_to_address_string
              (_provider.local_endpoint()).first;
          }

          int JobServer::port() const
          {
            return gspc::util::connectable_to_address_string
              (_provider.local_endpoint()).second;
          }

          void JobServer::wait (std::size_t N, State state)
          {
            return gspc::util::visit
              ( state
              , [&] (Running) { return _running.wait (N); }
              , [&] (Cancelled) { return _cancelled.wait (N); }
              , [&] (ExitedOrCancelled) { return _exited_or_cancelled.wait (N); }
              );
          }

          void JobServer::wait (std::size_t N, EurekaGroup eureka_group, State state)
          {
            return gspc::util::visit
              ( state
              , [&] (Running) { return _running.wait (N, eureka_group); }
              , [&] (Cancelled) { return _cancelled.wait (N, eureka_group); }
              , [&] (ExitedOrCancelled) { return _exited_or_cancelled.wait (N, eureka_group); }
              );
          }

          void JobServer::wait (std::size_t N, TasksByEurekaGroup expected, State state)
          {
            wait (N, state);
            GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION
              (expected, tasks (state));
          }

          TasksByEurekaGroup JobServer::tasks (State state)
          {
            return gspc::util::visit
              ( state
              , [&] (Running) { return _running.tasks(); }
              , [&] (Cancelled) { return _cancelled.tasks(); }
              , [&] (ExitedOrCancelled) { return _exited_or_cancelled.tasks(); }
              );
          }

          void JobServer::running (Task task, EurekaGroup eureka_group)
          {
            _running.add (task, eureka_group);
          }

          void JobServer::cancelled (Task task, EurekaGroup eureka_group)
          {
            _cancelled.add (task, eureka_group);
          }

          void JobServer::exited_or_cancelled (Task task, EurekaGroup eureka_group)
          {
            _exited_or_cancelled.add (task, eureka_group);
          }

          void JobServer::Tasks::add (Task task, EurekaGroup eureka_group)
          {
            std::lock_guard<std::mutex> const lock (_guard);
            _tasks_by_eureka_group[eureka_group].emplace (task);
            ++_N;
            _added.notify_all();
          }

          void JobServer::Tasks::wait (std::size_t N)
          {
            std::unique_lock<std::mutex> lock (_guard);
            _added.wait (lock, [&] { return _N >= N; });
          }

          void JobServer::Tasks::wait (std::size_t N, EurekaGroup eureka_group)
          {
            std::unique_lock<std::mutex> lock (_guard);
            _added.wait
              ( lock
              , [&]
                {
                  return N == 0
                    || (  _tasks_by_eureka_group.count (eureka_group)
                       && _tasks_by_eureka_group.at (eureka_group).size() >= N
                       )
                    ;
                }
              );
          }

          TasksByEurekaGroup JobServer::Tasks::tasks()
          {
            std::lock_guard<std::mutex> const lock (_guard);
            return _tasks_by_eureka_group;
          }
        }
