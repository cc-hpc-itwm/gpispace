// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/vmem/dtmmgr.hpp>

#include <stdexcept>

namespace iml_client
{
  namespace vmem
  {
    namespace
    {
      constexpr dtmmgr::Arena_t other[2]
        = {dtmmgr::ARENA_DOWN, dtmmgr::ARENA_UP};
    }

    dtmmgr::dtmmgr (MemSize_t MemSize, Align_t Align)
      : _arena {{{MemSize, Align}, {MemSize, Align}}}
      , _mem_size (_arena[ARENA_UP].memsize())
    {}

    std::pair<Offset_t, MemSize_t> dtmmgr::alloc
      (Handle_t Handle, Arena_t Arena, MemSize_t Size)
    {
      std::pair<Offset_t, MemSize_t> OffsetSize
        (_arena[Arena].alloc (Handle, Size));
      _arena[other[Arena]].resize (_mem_size - _arena[Arena].highwater());

      // invert for the local arena
      if (Arena == ARENA_DOWN)
      {
        if (! (_mem_size >= OffsetSize.first + OffsetSize.second))
        {
          throw std::logic_error
            ( "[dtmmgr] assertion '_mem_size >= OffsetSize.first + "
              "OffsetSize.second' failed."
            );
        }

        OffsetSize.first = _mem_size - (OffsetSize.first + OffsetSize.second);
      }

      return OffsetSize;
    }

    void dtmmgr::free (Handle_t Handle, Arena_t Arena)
    {
      _arena[Arena].free (Handle);
      _arena[other[Arena]].resize (_mem_size - _arena[Arena].highwater());
    }
  }
}
