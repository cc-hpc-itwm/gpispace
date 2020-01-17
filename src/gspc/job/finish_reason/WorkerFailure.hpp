#pragma once

#include <exception>

namespace gspc
{
  namespace job
  {
    namespace finish_reason
    {
      struct WorkerFailure
      {
        std::exception_ptr exception;

        template<typename Archive>
          void serialize (Archive& ar, unsigned int);
      };
    }
  }
}

#include <gspc/job/finish_reason/WorkerFailure.ipp>
