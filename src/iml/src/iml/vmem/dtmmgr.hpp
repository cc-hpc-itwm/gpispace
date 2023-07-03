// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

/* double table for two arenas, a global, growing upwards and a local
   growing downwards

   interface like in tmmgr, but each handle has to specify an arena
*/

#pragma once

#include <iml/vmem/tmmgr.hpp>

#include <array>

namespace iml_client
{
  namespace vmem
  {
    class dtmmgr
    {
    public:
      dtmmgr (MemSize_t, Align_t);

      enum Arena_t
        { ARENA_UP = 0,
          ARENA_DOWN
        };

      std::pair<Offset_t, MemSize_t> alloc (Handle_t, Arena_t, MemSize_t);
      void free (Handle_t, Arena_t);

      MemSize_t memfree() const
      {
        return std::max ( _arena[ARENA_UP].memfree()
                        , _arena[ARENA_DOWN].memfree()
                        );
      }
      Count_t numhandle (Arena_t Arena) const
      {
        return _arena[Arena].numhandle();
      }

    private:
      std::array<tmmgr, 2> _arena;
      MemSize_t _mem_size;
    };
  }
}
