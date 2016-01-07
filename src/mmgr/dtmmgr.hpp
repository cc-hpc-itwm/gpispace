
/* double table for two arenas, a global, growing upwards and a local
   growing downwards

   interface like in tmmgr, but each handle has to specify an arena
*/

#pragma once

#include <mmgr/tmmgr.hpp>
#include <vector>

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

      tmmgr::AllocReturn_t alloc (Handle_t, Arena_t, MemSize_t);
      tmmgr::HandleReturn_t free (Handle_t, Arena_t);
      tmmgr::HandleReturn_t offset_size
        (Handle_t, Arena_t, Offset_t*, MemSize_t*);

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
      std::vector<tmmgr> _arena;
      MemSize_t _mem_size;
    };
  }
}
