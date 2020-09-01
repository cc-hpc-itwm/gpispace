#pragma once

#include <boost/variant.hpp>

namespace gspc
{
  class vmem_allocation;

  namespace vmem
  {
    struct gaspi_segment_description;
    struct beegfs_segment_description;
    using segment_description = boost::variant < gaspi_segment_description
                                               , beegfs_segment_description
                                               >;
  }
}
