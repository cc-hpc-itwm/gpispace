#pragma once

#include <gspc/comm/worker/scheduler/protocol.hpp>
#include <gspc/rpc/TODO.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

namespace gspc
{
  namespace comm
  {
    namespace worker
    {
      namespace scheduler
      {
        struct Server
        {
        private:
          rpc::service_dispatcher _service_dispatcher;
          fhg::util::scoped_boost_asio_io_service_with_threads _io_service;

        public:
          rpc::service_handler<scheduler::finished> const _finished;

        private:
          rpc::service_socket_provider const _service_socket_provider;
          rpc::service_tcp_provider const _service_tcp_provider;
          rpc::endpoint const _local_endpoint;

        public:
          template<typename Finished>
            Server (Finished&&);
          //! \todo No template by providing a interface class?
          template<typename That>
            Server (That*);

          rpc::endpoint local_endpoint() const;
        };
      }
    }
  }
}

#include <gspc/comm/worker/scheduler/Server.ipp>
