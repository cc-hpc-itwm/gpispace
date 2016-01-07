#include <mmgr/dtmmgr.hpp>

#include <cassert>

namespace gspc
{
  namespace vmem
  {
    namespace
    {
      constexpr dtmmgr::Arena_t other[2]
        = {dtmmgr::ARENA_DOWN, dtmmgr::ARENA_UP};
    }

    dtmmgr::dtmmgr (MemSize_t MemSize, Align_t Align)
      : _arena (2, {MemSize, Align})
      , _mem_size (_arena[ARENA_UP].memsize())
    {}

    tmmgr::AllocReturn_t dtmmgr::alloc
      (Handle_t Handle, Arena_t Arena, MemSize_t Size)
    {
      tmmgr::AllocReturn_t const AllocReturn
        (_arena[Arena].alloc (Handle, Size));

      if (AllocReturn == tmmgr::ALLOC_SUCCESS)
      {
        _arena[other[Arena]].resize (_mem_size - _arena[Arena].highwater());
      }

      return AllocReturn;
    }

    tmmgr::HandleReturn_t dtmmgr::free (Handle_t Handle, Arena_t Arena)
    {
      tmmgr::HandleReturn_t const HandleReturn (_arena[Arena].free (Handle));

      if (HandleReturn == tmmgr::RET_SUCCESS)
      {
        _arena[other[Arena]].resize (_mem_size - _arena[Arena].highwater());
      }

      return HandleReturn;
    }

    tmmgr::HandleReturn_t dtmmgr::offset_size
      (Handle_t Handle, Arena_t Arena, Offset_t* POffset, MemSize_t* PMemSize)
    {
      MemSize_t Size (0);

      tmmgr::HandleReturn_t const HandleReturn
        (_arena[Arena].offset_size (Handle, POffset, &Size));

      if (HandleReturn == tmmgr::RET_SUCCESS)
      {
        // invert for the local arena
        if (Arena == ARENA_DOWN && POffset != nullptr)
        {
          assert (_mem_size >= *POffset + Size);

          *POffset = _mem_size - (*POffset + Size);
        }

        if (PMemSize != nullptr)
        {
          *PMemSize = Size;
        }
      }

      return HandleReturn;
    }
  }
}
