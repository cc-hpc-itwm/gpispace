#pragma once

namespace gpi
{
  namespace pc
  {
    enum flags_type
      {
        F_NONE         = 0x00

      , F_EXCLUSIVE    = 0x002 // no mapping possible from other processes
      , F_GLOBAL       = 0x004
      , F_NOCREATE     = 0x008 // do not create the segment (try to open it)
      };
  }
}