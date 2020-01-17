#include <gspc/comm/runtime_system/remote_interface/Client.hpp>

namespace gspc
{
  namespace comm
  {
    namespace runtime_system
    {
      namespace remote_interface
      {
        Client::Client
            (boost::asio::io_service& io_service, rpc::endpoint endpoint)
          : _endpoint {rpc::make_endpoint (io_service, std::move (endpoint))}
          , add {*_endpoint}
          , remove {*_endpoint}
          , worker_endpoint_for_scheduler {*_endpoint}
        {}
      }
    }
  }
}
