#pragma once

namespace gpi
{
  namespace pc
  {
    enum flags_type
      {
        F_EXCLUSIVE    = 0x002 // no mapping possible from other processes
      , F_GLOBAL       = 0x004
      };
  }
}
