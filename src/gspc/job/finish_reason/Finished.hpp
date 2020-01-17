#pragma once

#include <gspc/ErrorOr.hpp>
#include <gspc/task/Result.hpp>

namespace gspc
{
  namespace job
  {
    namespace finish_reason
    {
      struct Finished
      {
        //! \todo multiple tasks per job!?
        ErrorOr<task::Result> task_result;

        template<typename Archive>
          void serialize (Archive& ar, unsigned int);
      };
    }
  }
}

#include <gspc/job/finish_reason/Finished.ipp>
