#ifndef GPI_SPACE_PC_TYPE_SEGMENT_TYPE_HPP
#define GPI_SPACE_PC_TYPE_SEGMENT_TYPE_HPP 1

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
          , SEG_SFS      = 3    // (parallel) shared file-system based segment
          , SEG_MAX_TYPE = ((1 << 4) - 1)
          };
      }
    }
  }
}

#endif
