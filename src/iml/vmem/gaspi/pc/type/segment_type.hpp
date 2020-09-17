#pragma once

#include <cstddef>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      namespace segment
      {
        enum segment_type : std::size_t
          {
            // maximum of 4 bits available --> see handle_t
            SEG_INVAL    = 0
          , SEG_GASPI    = 1    // GASPI based segment
          , SEG_SHM      = 2    // SHM based segment
          , SEG_BEEGFS   = 3    // (parallel) beegfs shared file-system based segment
          };
      }
    }
  }
}
