#pragma once

#include <boost/variant.hpp>

namespace gspc
{
  class scoped_vmem_segment_and_allocation;

  namespace vmem
  {
    struct gaspi_segment_description;
    struct beegfs_segment_description;
    using segment_description = boost::variant < gaspi_segment_description
                                               , beegfs_segment_description
                                               >;
  }
}
