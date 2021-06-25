// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <we/test/eureka/jobserver/client.hpp>
#include <we/test/eureka/jobserver/protocol.hpp>

#include <rpc/remote_function.hpp>
#include <rpc/remote_tcp_endpoint.hpp>

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
