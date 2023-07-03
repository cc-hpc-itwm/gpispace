// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/test/eureka/jobserver/client.hpp>
#include <we/test/eureka/jobserver/protocol.hpp>

#include <util-rpc/remote_function.hpp>
#include <util-rpc/remote_tcp_endpoint.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

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
          struct client::implementation
          {
            implementation ( std::string host
                           , unsigned short port
                           , Task task
                           , EurekaGroup eureka_group
                           )
              : _task (task)
              , _eureka_group (eureka_group)
              , _io_service (1)
              , _endpoint (_io_service, host, port)
              , _cancelled (_endpoint)
              , _exited_or_cancelled (_endpoint)
              , _running (_endpoint)
            {}

            void running()
            {
              return _running (_task, _eureka_group);
            }

            void cancelled()
            {
              return _cancelled (_task, _eureka_group);
            }

            void exited_or_cancelled()
            {
              return _exited_or_cancelled (_task, _eureka_group);
            }

          private:
            Task _task;
            EurekaGroup _eureka_group;
            fhg::util::scoped_boost_asio_io_service_with_threads _io_service;
            fhg::rpc::remote_tcp_endpoint _endpoint;
            fhg::rpc::sync_remote_function<protocol::cancelled> _cancelled;
            fhg::rpc::sync_remote_function<protocol::exited_or_cancelled>
              _exited_or_cancelled;
            fhg::rpc::sync_remote_function<protocol::running> _running;
          };

          client::client ( std::string host
                         , unsigned short port
                         , Task task
                         , EurekaGroup eureka_group
                         )
            : _implementation
              (new implementation (host, port, task, eureka_group))
          {
            _implementation->running();
          }

          void client::cancelled()
          {
            return _implementation->cancelled();
          }

          void client::exited_or_cancelled()
          {
            return _implementation->exited_or_cancelled();
          }

          client::~client() = default;
        }
      }
    }
  }
}
