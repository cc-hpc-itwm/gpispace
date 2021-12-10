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

#pragma once

#include <util-rpc/remote_function.hpp>
#include <util-rpc/remote_tcp_endpoint.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <utility>

namespace gspc
{
  namespace test
  {
    namespace parallel_reduce
    {
      namespace module_call
      {
        namespace remote_function
        {
          template<typename F> class Client
          {
          private:
            fhg::util::scoped_boost_asio_io_service_with_threads
              _io_service {1};
            fhg::rpc::remote_tcp_endpoint _endpoint;
            fhg::rpc::sync_remote_function<F> _function {_endpoint};

          public:
            template<typename... Args>
              Client (Args&&... args)
                : _endpoint {_io_service, std::forward<Args> (args)...}
            {}

            template<typename... Args>
              typename F::result_type operator() (Args&&... args)
            {
              return _function (std::forward<Args> (args)...);
            }
          };
        }
      }
    }
  }
}
