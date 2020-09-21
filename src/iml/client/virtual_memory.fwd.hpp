#pragma once

#include <iml/segment_description.hpp>

namespace iml_client
{
  class vmem_allocation;

  namespace vmem
  {
    using gaspi_segment_description = iml::gaspi_segment_description;
    using beegfs_segment_description = iml::beegfs_segment_description;
    using segment_description = iml::segment_description;
  }
}
