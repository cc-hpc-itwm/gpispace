// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
