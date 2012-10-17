#ifndef GPI_SPACE_PC_TYPE_FLAGS_HPP
#define GPI_SPACE_PC_TYPE_FLAGS_HPP

namespace gpi
{
  namespace pc
  {
    enum flags_type
      {
        F_NONE         = 0x00

      , F_PERSISTENT   = 0x001 // leave segment in gpi after process death
      , F_EXCLUSIVE    = 0x002 // no mapping possible from other processes
      , F_GLOBAL       = 0x004
      , F_NOCREATE     = 0x008 // do not create the segment (try to open it)
      , F_FORCE_UNLINK = 0x010 // force recreation of the segment
      , F_SPECIAL      = 0x020 // special segment (used internally to identify local/global segments)
      , F_ATTACHED     = 0x040 // special flag indicating if the process container is attached
      , F_NOMMAP       = 0x080 // do not mmap, rather use seek/read/write if possible
      , F_OWNER        = 0x100 // are we the owner?
      };
  }
}

#endif
