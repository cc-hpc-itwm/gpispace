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

    void dtmmgr::free (Handle_t Handle, Arena_t Arena)
    {
      _arena[Arena].free (Handle);
      _arena[other[Arena]].resize (_mem_size - _arena[Arena].highwater());
    }

    std::pair<Offset_t, MemSize_t> dtmmgr::offset_size
      (Handle_t Handle, Arena_t Arena) const
    {
      std::pair<Offset_t, MemSize_t> OffsetSize
        (_arena[Arena].offset_size (Handle));

      // invert for the local arena
      if (Arena == ARENA_DOWN)
      {
        assert (_mem_size >= OffsetSize.first + OffsetSize.second);

        OffsetSize.first = _mem_size - (OffsetSize.first + OffsetSize.second);
      }

      return OffsetSize;
    }
  }
}
