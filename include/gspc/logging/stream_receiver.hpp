// Copyright (C) 2018-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/logging/endpoint.hpp>
#include <gspc/logging/message.hpp>
#include <gspc/logging/protocol.hpp>

#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_socket_provider.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>

#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>

#include <functional>
#include <string>
#include <utility>
#include <vector>


  namespace gspc::logging
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
      gspc::rpc::service_dispatcher _service_dispatcher;
      gspc::util::scoped_boost_asio_io_service_with_threads _io_service;
      gspc::rpc::service_handler<protocol::receive> const _receive;
      gspc::rpc::service_tcp_provider const _service_tcp_provider;
      gspc::rpc::service_socket_provider const _service_socket_provider;
      endpoint const _local_endpoint;
    };
  }
