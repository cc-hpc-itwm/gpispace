#pragma once

#include <iml/client/virtual_memory.fwd.hpp>

namespace gspc
{
  class vmem_allocation;

  namespace vmem
  {
    using beegfs_segment_description = iml_client::vmem::beegfs_segment_description;
    using gaspi_segment_description = iml_client::vmem::gaspi_segment_description;

    using segment_description = iml_client::vmem::segment_description;
  }
}
