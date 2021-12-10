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
