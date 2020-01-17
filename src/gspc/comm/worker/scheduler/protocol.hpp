#pragma once

#include <gspc/job/FinishReason.hpp>
#include <gspc/job/ID.hpp>
#include <gspc/rpc/TODO.hpp>

namespace gspc
{
  namespace comm
  {
    namespace worker
    {
      namespace scheduler
      {
        FHG_RPC_FUNCTION_DESCRIPTION
          ( finished
          , void (job::ID, job::FinishReason)
          );
      }
    }
  }
}
