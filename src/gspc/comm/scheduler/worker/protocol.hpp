#pragma once

#include <gspc/Job.hpp>
#include <gspc/job/ID.hpp>
#include <gspc/rpc/TODO.hpp>

namespace gspc
{
  namespace comm
  {
    namespace scheduler
    {
      namespace worker
      {
        //! \todo worker::scheduler::Server::endpoint?
        FHG_RPC_FUNCTION_DESCRIPTION
          ( submit
          , void (rpc::endpoint, job::ID, Job)
          );
        FHG_RPC_FUNCTION_DESCRIPTION
          ( cancel
          , void (job::ID)
          );
        // status
      }
    }
  }
}
