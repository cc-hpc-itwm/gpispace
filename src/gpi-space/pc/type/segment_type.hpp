#pragma once

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      namespace segment
      {
        enum segment_type
          {
            // maximum of 4 bits available --> see handle_t
            SEG_INVAL    = 0
          , SEG_GPI      = 1    // GPI based segment
          , SEG_SHM      = 2    // SHM based segment
          , SEG_BEEGFS   = 3    // (parallel) beegfs shared file-system based segment
          , SEG_MAX_TYPE = ((1 << 4) - 1)
          };
      }
    }
  }
}
