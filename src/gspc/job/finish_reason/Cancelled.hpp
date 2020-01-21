#pragma once

#include <gspc/task/Result.hpp>

namespace gspc
{
  namespace job
  {
    namespace finish_reason
    {
      struct Cancelled
      {
        task::result::Premature reason;

        template<typename Archive>
          void serialize (Archive& ar, unsigned int);
      };
    }
  }
}

#include <gspc/job/finish_reason/Cancelled.ipp>
