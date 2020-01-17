#pragma once

#include <gspc/comm/worker/scheduler/protocol.hpp>
#include <gspc/rpc/TODO.hpp>

#include <boost/asio/io_service.hpp>

#include <memory>

namespace gspc
{
  namespace comm
  {
    namespace worker
    {
      namespace scheduler
      {
        struct Client
        {
        public:
          Client (boost::asio::io_service&, rpc::endpoint);

        private:
          std::unique_ptr<rpc::remote_endpoint> _endpoint;

        public:
          rpc::sync_remote_function<scheduler::finished> finished;
        };
      }
    }
  }
}
