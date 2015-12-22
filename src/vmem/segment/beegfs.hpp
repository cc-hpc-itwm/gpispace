#pragma once

#include <stdexcept>

namespace fhg
{
  namespace vmem
  {
    namespace segment
    {
      namespace beegfs
      {
        struct requirements_not_met : std::runtime_error
        {
          requirements_not_met (std::string const& reason)
            : std::runtime_error (reason)
          {}
        };

        void check_requirements (int fd);
      }
    }
  }
}
