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

#include <logging/endpoint.hpp>
#include <logging/message.hpp>
#include <logging/protocol.hpp>

#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>
#include <util-rpc/service_socket_provider.hpp>
#include <util-rpc/service_tcp_provider.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace fhg
{
  namespace logging
  {
    class stream_receiver
    {
    public:
      //! \note The callback shall not be blocking and shall not use
      //! an `io_service` sharing anything with `this->_io_service`.
      using callback_t = std::function<void (message const&)>;
      using yielding_callback_t
        = std::function<void (::boost::asio::yield_context, message const&)>;
      stream_receiver (callback_t);
      stream_receiver (yielding_callback_t);
      stream_receiver (endpoint, callback_t);
      stream_receiver (std::vector<endpoint>, callback_t);

      void add_emitters (::boost::asio::yield_context, std::vector<endpoint>);
      void add_emitters_blocking (std::vector<endpoint>);

    private:
      rpc::service_dispatcher _service_dispatcher;
      util::scoped_boost_asio_io_service_with_threads _io_service;
      rpc::service_handler<protocol::receive> const _receive;
      rpc::service_tcp_provider const _service_tcp_provider;
      rpc::service_socket_provider const _service_socket_provider;
      endpoint const _local_endpoint;
    };
  }
}
