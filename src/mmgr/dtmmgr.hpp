// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

/* double table for two arenas, a global, growing upwards and a local
   growing downwards

   interface like in tmmgr, but each handle has to specify an arena
*/

#pragma once

#include <mmgr/tmmgr.hpp>
#include <array>

namespace gspc
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
