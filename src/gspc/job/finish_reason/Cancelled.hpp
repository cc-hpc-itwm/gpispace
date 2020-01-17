#pragma once

namespace gspc
{
  namespace job
  {
    namespace finish_reason
    {
      struct Cancelled
      {
        template<typename Archive>
          void serialize (Archive& ar, unsigned int);
      };
    }
  }
}

#include <gspc/job/finish_reason/Cancelled.ipp>
