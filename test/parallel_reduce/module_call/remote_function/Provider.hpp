// Copyright (C) 2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>

#include <string>
#include <utility>





        namespace gspc::test::parallel_reduce::module_call::remote_function
        {
          template<typename F> class Provider
          {
          private:
            gspc::rpc::service_dispatcher _dispatcher{};
            gspc::rpc::service_handler<F> _function;
            gspc::util::scoped_boost_asio_io_service_with_threads
              _io_service {1};
            gspc::rpc::service_tcp_provider _provider
              {_io_service, _dispatcher};

          public:
            template<typename... Args>
              Provider (Args&&... args)
                : _function (_dispatcher, std::forward<Args> (args)...)
            {}

            std::pair<std::string, unsigned short> address() const
            {
              return gspc::util::connectable_to_address_string
                (_provider.local_endpoint())
                ;
            }
          };
        }
