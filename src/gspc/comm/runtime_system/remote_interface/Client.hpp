#pragma once

#include <gspc/comm/runtime_system/remote_interface/protocol.hpp>
#include <gspc/rpc/TODO.hpp>

#include <boost/asio/io_service.hpp>

#include <memory>

namespace gspc
{
  namespace comm
  {
    namespace runtime_system
    {
      namespace remote_interface
      {
        //! \todo syntax goal:
        //! RPC_CLIENT (Client, add, remove, worker_endpoint_for_scheduler);
        //! RPC_SERVER (Server, add, remove, worker_endpoint_for_scheduler);
        struct Client
        {
        public:
          Client (boost::asio::io_service&, rpc::endpoint);

        private:
          std::unique_ptr<rpc::remote_endpoint> _endpoint;

        public:
          rpc::sync_remote_function<remote_interface::add> add;
          rpc::sync_remote_function<remote_interface::remove> remove;
          rpc::sync_remote_function
            <remote_interface::worker_endpoint_for_scheduler>
              worker_endpoint_for_scheduler;
        };
      }
    }
  }
}
