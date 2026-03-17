// Copyright (C) 2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/rpc/remote_function.hpp>
#include <gspc/rpc/remote_tcp_endpoint.hpp>

#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>

#include <utility>





        namespace gspc::test::parallel_reduce::module_call::remote_function
        {
          template<typename F> class Client
          {
          private:
            gspc::util::scoped_boost_asio_io_service_with_threads
              _io_service {1};
            gspc::rpc::remote_tcp_endpoint _endpoint;
            gspc::rpc::sync_remote_function<F> _function {_endpoint};

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
