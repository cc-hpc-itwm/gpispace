#include <gspc/comm/worker/scheduler/Server.hpp>

namespace gspc
{
  namespace comm
  {
    namespace worker
    {
      namespace scheduler
      {
        rpc::endpoint Server::local_endpoint() const
        {
          return _local_endpoint;
        }
      }
    }
  }
}
