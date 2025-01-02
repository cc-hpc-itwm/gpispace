// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>
#include <util-rpc/service_tcp_provider.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

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
        namespace remote_function
        {
          template<typename F> class Provider
          {
          private:
            fhg::rpc::service_dispatcher _dispatcher{};
            fhg::rpc::service_handler<F> _function;
            fhg::util::scoped_boost_asio_io_service_with_threads
              _io_service {1};
            fhg::rpc::service_tcp_provider _provider
              {_io_service, _dispatcher};

          public:
            template<typename... Args>
              Provider (Args&&... args)
                : _function (_dispatcher, std::forward<Args> (args)...)
            {}

            std::pair<std::string, unsigned short> address() const
            {
              return fhg::util::connectable_to_address_string
                (_provider.local_endpoint())
                ;
            }
          };
        }
      }
    }
  }
}
