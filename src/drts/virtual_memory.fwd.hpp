#pragma once

#include <iml/client/virtual_memory.fwd.hpp>

namespace gspc
{
  class vmem_allocation;

  namespace vmem
  {
    using beegfs_segment_description = iml::beegfs_segment_description;
    using gaspi_segment_description = iml::gaspi_segment_description;

    using segment_description = iml::segment_description;
  }
}
