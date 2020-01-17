#include <gspc/comm/scheduler/worker/Server.hpp>

namespace gspc
{
  namespace comm
  {
    namespace scheduler
    {
      namespace worker
      {
        rpc::endpoint Server::local_endpoint() const
        {
          return _local_endpoint;
        }
      }
    }
  }
}
