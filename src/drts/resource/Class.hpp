#pragma once

#include <string>

namespace gspc
{
  namespace resource
  {
    //! \note *not* a set, if users want a single worker to have more
    //! than one resource, they need to explicitly model that in the
    //! forest. (e.g. "CPU+Compute" -> ["CPU", "Compute"], which
    //! allows for aquisition of "CPU", "Compute" or "CPU+Compute",
    //! and allows the user to specify exactly which subset is a valid
    //! aquisition, e.g. "GPU+Compute" -> ["Compute"], would not allow
    //! to aquire a solo GPU worker, but still a compute-only task).
    using Class = std::string;
  }
}
