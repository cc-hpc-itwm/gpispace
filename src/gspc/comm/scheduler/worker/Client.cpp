#include <gspc/comm/scheduler/worker/Client.hpp>

namespace gspc
{
  namespace comm
  {
    namespace scheduler
    {
      namespace worker
      {
        Client::Client
            (boost::asio::io_service& io_service, rpc::endpoint endpoint)
          : _endpoint {rpc::make_endpoint (io_service, std::move (endpoint))}
          , submit {*_endpoint}
          , cancel {*_endpoint}
        {}
      }
    }
  }
}
