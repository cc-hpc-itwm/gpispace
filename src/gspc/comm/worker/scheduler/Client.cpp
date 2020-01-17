#include <gspc/comm/worker/scheduler/Client.hpp>

namespace gspc
{
  namespace comm
  {
    namespace worker
    {
      namespace scheduler
      {
        Client::Client
            (boost::asio::io_service& io_service, rpc::endpoint endpoint)
          : _endpoint {rpc::make_endpoint (io_service, std::move (endpoint))}
          , finished {*_endpoint}
        {}
      }
    }
  }
}
