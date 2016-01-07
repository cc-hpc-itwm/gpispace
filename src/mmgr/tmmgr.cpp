
#include <mmgr/tmmgr.h>

#include <util-generic/unused.hpp>

#include <assert.h>

#include <map>
#include <unordered_map>
#include <unordered_set>

typedef struct
{
  std::unordered_map<Handle_t, std::pair<Offset_t, MemSize_t>>
    handle_to_offset_and_size;
  std::unordered_map<Offset_t, Handle_t> offset_to_handle;
  std::map<MemSize_t, std::unordered_multiset<Offset_t>>
    free_offset_by_size;
  std::unordered_map<Offset_t, Size_t> free_segment_start;
  std::unordered_map<Offset_t, Size_t> free_segment_end;
  MemSize_t mem_size;
  MemSize_t mem_free;
  MemSize_t high_water;
  Align_t align;
} tmmgr_t, *ptmmgr_t;

static inline MemSize_t
alignDown (MemSize_t Size, Align_t Align)
{
  return (Size & ~(Align - 1));
}

static inline Offset_t
alignUp (MemSize_t Size, Align_t Align)
{
  return alignDown (Size + Align - 1, Align);
}

static void
tmmgr_del_free_seg (ptmmgr_t ptmmgr, const Offset_t OffsetStart,
                    const MemSize_t Size)
{
  Offset_t OffsetEnd = OffsetStart + Size;

  {
    auto pos_size (ptmmgr->free_offset_by_size.find (Size));
    if (pos_size == ptmmgr->free_offset_by_size.end())
    {
      throw std::runtime_error ("Missing Size " + std::to_string (Size));
    }
    auto& offsets (pos_size->second);
    auto pos_offset (offsets.find (OffsetStart));
    if (pos_offset == offsets.end())
    {
      throw std::logic_error ("Missing Offset " + std::to_string (OffsetStart));
    }
    offsets.erase (pos_offset);
    if (offsets.empty())
    {
      ptmmgr->free_offset_by_size.erase (pos_size);
    }
  }
  ptmmgr->free_segment_start.erase (OffsetStart);
  ptmmgr->free_segment_end.erase (OffsetEnd);
}

static void
tmmgr_ins_free_seg (ptmmgr_t ptmmgr, const Offset_t OffsetStart,
                    const MemSize_t Size)
{
  if (Size > 0)
    {
      Offset_t OffsetEnd = OffsetStart + Size;

      ptmmgr->free_offset_by_size[Size].emplace (OffsetStart);
      ptmmgr->free_segment_start.emplace (OffsetStart, Size);
      ptmmgr->free_segment_end.emplace (OffsetEnd, Size);
    }
}

MemSize_t
tmmgr_init (PTmmgr_t PTmmgr, const MemSize_t MemSize, const Align_t Align)
{
  if (PTmmgr == nullptr || *(ptmmgr_t *) PTmmgr != nullptr)
    return 0;

  *(ptmmgr_t *) PTmmgr = new tmmgr_t;

  ptmmgr_t ptmmgr = *(ptmmgr_t *) PTmmgr;

  if (ptmmgr == nullptr)
    TMMGR_ERROR_MALLOC_FAILED;

  ptmmgr->align = Align;

  ptmmgr->mem_size = ptmmgr->mem_free =
    alignDown (MemSize, ptmmgr->align);
  ptmmgr->high_water = 0;

  tmmgr_ins_free_seg (ptmmgr, 0, ptmmgr->mem_size);

  return ptmmgr->mem_size;
}

ResizeReturn_t
tmmgr_resize (PTmmgr_t PTmmgr, const MemSize_t NewSizeUnaligned)
{
  if (PTmmgr == nullptr || *(ptmmgr_t *) PTmmgr == nullptr)
    return RESIZE_FAILURE;

  ptmmgr_t ptmmgr = *(ptmmgr_t *) PTmmgr;

  const MemSize_t NewSize = alignDown (NewSizeUnaligned, ptmmgr->align);

  if (NewSize < ptmmgr->mem_size - ptmmgr->mem_free)
    return RESIZE_BELOW_MEMUSED;

  if (NewSize < ptmmgr->high_water)
    return RESIZE_BELOW_HIGHWATER;

  MemSize_t OldSize = ptmmgr->mem_size;

  if (NewSize == OldSize)
    return RESIZE_SUCCESS;

  MemSize_t Delta =
    (NewSize > OldSize) ? (NewSize - OldSize) : (OldSize - NewSize);

  auto free_segment_end (ptmmgr->free_segment_end.find (OldSize));

  if (free_segment_end != ptmmgr->free_segment_end.end())
    {
      /* longer a free segment */

      MemSize_t FreeSize = free_segment_end->second;

      assert (NewSize > OldSize || Delta <= FreeSize);

      MemSize_t NewFreeSize =
        (NewSize > OldSize) ? (FreeSize + Delta) : (FreeSize - Delta);

      Offset_t OffsetStart = OldSize - FreeSize;

      tmmgr_del_free_seg (ptmmgr, OffsetStart, FreeSize);
      tmmgr_ins_free_seg (ptmmgr, OffsetStart, NewFreeSize);
    }
  else
    {
      /* add a new free segment */

      assert (NewSize > OldSize);

      tmmgr_ins_free_seg (ptmmgr, (Offset_t) OldSize, Delta);
    }

  ptmmgr->mem_size = NewSize;
  ptmmgr->mem_free =
    (NewSize >
     OldSize) ? (ptmmgr->mem_free + Delta) : (ptmmgr->mem_free - Delta);

  return RESIZE_SUCCESS;
}

MemSize_t
tmmgr_finalize (PTmmgr_t PTmmgr)
{
  if (PTmmgr == nullptr || *(ptmmgr_t *) PTmmgr == nullptr)
    return 0;

  ptmmgr_t ptmmgr = *(ptmmgr_t *) PTmmgr;
  MemSize_t Bytes = sizeof (tmmgr_t);

  delete ptmmgr;

  *(ptmmgr_t *) PTmmgr = nullptr;

  return Bytes;
}

HandleReturn_t
tmmgr_offset_size (const Tmmgr_t Tmmgr, const Handle_t Handle,
                   POffset_t POffset, PMemSize_t PMemSize)
{
  if (Tmmgr == nullptr)
    return RET_FAILURE;

  ptmmgr_t ptmmgr = static_cast<ptmmgr_t> (Tmmgr);

  auto pos (ptmmgr->handle_to_offset_and_size.find (Handle));

  if (pos == ptmmgr->handle_to_offset_and_size.end())
  {
    return RET_HANDLE_UNKNOWN;
  }

  if (POffset)
  {
    *POffset = pos->second.first;
  }
  if (PMemSize)
  {
    *PMemSize = pos->second.second;
  }

  return RET_SUCCESS;
}

static void
tmmgr_ins (ptmmgr_t ptmmgr, const Handle_t Handle, const Offset_t Offset,
           const MemSize_t Size)
{
  assert (Size > 0);

  assert (Size <= ptmmgr->mem_free);

  ptmmgr->mem_free -= Size;

  if (ptmmgr->high_water < Offset + Size)
    ptmmgr->high_water = Offset + Size;

  auto PSizeStart (ptmmgr->free_segment_start.find (Offset));

  assert (PSizeStart != ptmmgr->free_segment_start.end());
  assert (Size <= PSizeStart->second);

#ifndef NDEBUG
  auto free_segment_end
    (ptmmgr->free_segment_end.find (Offset + PSizeStart->second));
#endif

  assert (free_segment_end != ptmmgr->free_segment_end.end());
  assert (PSizeStart->second == free_segment_end->second);

  Size_t OldFreeSize = PSizeStart->second;
  Size_t NewFreeSize = PSizeStart->second - Size;

  tmmgr_del_free_seg (ptmmgr, Offset, OldFreeSize);
  tmmgr_ins_free_seg (ptmmgr, Offset + Size, NewFreeSize);

  ptmmgr->handle_to_offset_and_size.emplace
    (Handle, std::make_pair (Offset, Size));
  ptmmgr->offset_to_handle.emplace (Offset, Handle);
}

AllocReturn_t
tmmgr_alloc (PTmmgr_t PTmmgr, const Handle_t Handle,
             const MemSize_t SizeUnaligned)
{
  if (PTmmgr == nullptr || *(ptmmgr_t *) PTmmgr == nullptr)
    return ALLOC_FAILURE;

  ptmmgr_t ptmmgr = *(ptmmgr_t *) PTmmgr;

  const MemSize_t Size = alignUp (SizeUnaligned, ptmmgr->align);

  if (ptmmgr->mem_free < Size)
    return ALLOC_INSUFFICIENT_MEMORY;

  if (tmmgr_offset_size (ptmmgr, Handle, nullptr, nullptr) == RET_SUCCESS)
    return ALLOC_DUPLICATE_HANDLE;

  auto pos (ptmmgr->free_offset_by_size.lower_bound (Size));

  if (pos == ptmmgr->free_offset_by_size.end())
  {
    return ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY;
  }

  Offset_t Offset (*pos->second.begin());

  tmmgr_ins (ptmmgr, Handle, Offset, Size);

  return ALLOC_SUCCESS;
}

static void
tmmgr_del (ptmmgr_t ptmmgr, const Handle_t Handle, const Offset_t Offset,
           const MemSize_t Size)
{
  assert (Size > 0);

  ptmmgr->mem_free += Size;

  ptmmgr->handle_to_offset_and_size.erase (Handle);
  ptmmgr->offset_to_handle.erase (Offset);

  auto PSizeL (ptmmgr->free_segment_end.find (Offset));
  auto PSizeR (ptmmgr->free_segment_start.find (Offset + Size));

  if (Offset + Size == ptmmgr->high_water)
    {
      ptmmgr->high_water = Offset - ((PSizeL == ptmmgr->free_segment_end.end()) ? 0 : PSizeL->second);
    }

  if (  PSizeL != ptmmgr->free_segment_end.end()
     && PSizeR != ptmmgr->free_segment_start.end()
     )
    {
      /* join left and right segment */

      Size_t SizeL = PSizeL->second;
      Size_t SizeR = PSizeR->second;

      assert (Offset >= SizeL);

      Offset_t OffsetL = Offset - SizeL;
      Offset_t OffsetR = Offset + Size;

      tmmgr_del_free_seg (ptmmgr, OffsetL, SizeL);
      tmmgr_del_free_seg (ptmmgr, OffsetR, SizeR);
      tmmgr_ins_free_seg (ptmmgr, OffsetL, SizeL + Size + SizeR);
    }
  else if (PSizeL != ptmmgr->free_segment_end.end())
    {
      /* longer the left segment */

      MemSize_t SizeL = PSizeL->second;

      assert (Offset >= SizeL);

      Offset_t OffsetL = Offset - SizeL;

      tmmgr_del_free_seg (ptmmgr, OffsetL, SizeL);
      tmmgr_ins_free_seg (ptmmgr, OffsetL, SizeL + Size);
    }
  else if (PSizeR != ptmmgr->free_segment_start.end())
    {
      /* longer the right segment */

      MemSize_t SizeR = PSizeR->second;

      Offset_t OffsetR = Offset + Size;

      tmmgr_del_free_seg (ptmmgr, OffsetR, SizeR);
      tmmgr_ins_free_seg (ptmmgr, Offset, Size + SizeR);
    }
  else
    {
      tmmgr_ins_free_seg (ptmmgr, Offset, Size);
    }
}

HandleReturn_t
tmmgr_free (PTmmgr_t PTmmgr, const Handle_t Handle)
{
  if (PTmmgr == nullptr || *(ptmmgr_t *) PTmmgr == nullptr)
    return RET_FAILURE;

  ptmmgr_t ptmmgr = *(ptmmgr_t *) PTmmgr;

  Offset_t Offset;
  MemSize_t Size;

  if (tmmgr_offset_size (ptmmgr, Handle, &Offset, &Size) ==
      RET_HANDLE_UNKNOWN)
    return RET_HANDLE_UNKNOWN;

  tmmgr_del (ptmmgr, Handle, Offset, Size);

  return RET_SUCCESS;
}

MemSize_t
tmmgr_memsize (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == nullptr)
    return 0;

  ptmmgr_t ptmmgr = static_cast<ptmmgr_t> (Tmmgr);

  return ptmmgr->mem_size;
}

MemSize_t
tmmgr_memfree (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == nullptr)
    return 0;

  ptmmgr_t ptmmgr = static_cast<ptmmgr_t> (Tmmgr);

  return ptmmgr->mem_free;
}

MemSize_t
tmmgr_highwater (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == nullptr)
    return 0;

  ptmmgr_t ptmmgr = static_cast<ptmmgr_t> (Tmmgr);

  return ptmmgr->high_water;
}

Count_t
tmmgr_numhandle (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == nullptr)
    return 0;

  ptmmgr_t ptmmgr = static_cast<ptmmgr_t> (Tmmgr);

  return ptmmgr->handle_to_offset_and_size.size();
}
