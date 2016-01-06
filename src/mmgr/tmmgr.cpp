
#include <mmgr/tmmgr.h>

#include <mmgr/trie.h>
#include <mmgr/heap.hpp>

#include <util-generic/unused.hpp>

#include <assert.h>

#include <map>
#include <unordered_map>
#include <unordered_set>

typedef struct
{
  std::unordered_map<Handle_t, std::pair<Offset_t, MemSize_t>>
    handle_to_offset_and_size;
  TrieMap_t offset_to_handle;
  std::map<MemSize_t, std::unordered_multiset<Offset_t>>
    free_offset_by_size;
  TrieMap_t free_segment_start;
  TrieMap_t free_segment_end;
  MemSize_t mem_size;
  MemSize_t mem_free;
  MemSize_t min_free;
  MemSize_t high_water;
  Align_t align;
  Count_t num_alloc;
  Count_t num_free;
  MemSize_t sum_alloc;
  MemSize_t sum_free;
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
  trie_del (&ptmmgr->free_segment_start, OffsetStart, fUserNone);
  trie_del (&ptmmgr->free_segment_end, OffsetEnd, fUserNone);
}

static void
tmmgr_ins_free_seg (ptmmgr_t ptmmgr, const Offset_t OffsetStart,
                    const MemSize_t Size)
{
  if (Size > 0)
    {
      Offset_t OffsetEnd = OffsetStart + Size;

      ptmmgr->free_offset_by_size[Size].emplace (OffsetStart);
      *trie_ins (&ptmmgr->free_segment_start, OffsetStart, nullptr) = Size;
      *trie_ins (&ptmmgr->free_segment_end, OffsetEnd, nullptr) = Size;
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

  ptmmgr->offset_to_handle = (TrieMap_t) nullptr;
  ptmmgr->free_segment_start = (TrieMap_t) nullptr;
  ptmmgr->free_segment_end = (TrieMap_t) nullptr;

  ptmmgr->align = Align;

  ptmmgr->mem_size = ptmmgr->mem_free = ptmmgr->min_free =
    alignDown (MemSize, ptmmgr->align);
  ptmmgr->high_water = 0;
  ptmmgr->num_alloc = ptmmgr->num_free = 0;
  ptmmgr->sum_alloc = ptmmgr->sum_free = 0;

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
  PMemSize_t PFreeSize = trie_get (ptmmgr->free_segment_end, OldSize);

  if (PFreeSize != nullptr)
    {
      /* longer a free segment */

      MemSize_t FreeSize = *PFreeSize;

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

  if (ptmmgr->mem_free < ptmmgr->min_free)
    ptmmgr->min_free = ptmmgr->mem_free;

  return RESIZE_SUCCESS;
}

MemSize_t
tmmgr_finalize (PTmmgr_t PTmmgr)
{
  if (PTmmgr == nullptr || *(ptmmgr_t *) PTmmgr == nullptr)
    return 0;

  ptmmgr_t ptmmgr = *(ptmmgr_t *) PTmmgr;
  MemSize_t Bytes = sizeof (tmmgr_t);

  Bytes += trie_free (&ptmmgr->offset_to_handle, fUserNone);
  Bytes += trie_free (&ptmmgr->free_segment_start, fUserNone);
  Bytes += trie_free (&ptmmgr->free_segment_end, fUserNone);

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

  if (ptmmgr->mem_free < ptmmgr->min_free)
    ptmmgr->min_free = ptmmgr->mem_free;

  if (ptmmgr->high_water < Offset + Size)
    ptmmgr->high_water = Offset + Size;

  PMemSize_t PSizeStart = trie_get (ptmmgr->free_segment_start, Offset);

  assert (PSizeStart != nullptr);
  assert (Size <= *PSizeStart);

#ifndef NDEBUG
  PMemSize_t PSizeEnd =
#endif
    trie_get (ptmmgr->free_segment_end, Offset + (*PSizeStart));

  assert (PSizeEnd != nullptr);
  assert (*PSizeStart == *PSizeEnd);

  Size_t OldFreeSize = *PSizeStart;
  Size_t NewFreeSize = *PSizeStart - Size;

  tmmgr_del_free_seg (ptmmgr, Offset, OldFreeSize);
  tmmgr_ins_free_seg (ptmmgr, Offset + Size, NewFreeSize);

  ptmmgr->handle_to_offset_and_size.emplace
    (Handle, std::make_pair (Offset, Size));
  *trie_ins (&ptmmgr->offset_to_handle, Offset, nullptr) = Handle;
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

  ++ptmmgr->num_alloc;
  ptmmgr->sum_alloc += Size;

  return ALLOC_SUCCESS;
}

static void
tmmgr_del (ptmmgr_t ptmmgr, const Handle_t Handle, const Offset_t Offset,
           const MemSize_t Size)
{
  assert (Size > 0);

  ptmmgr->mem_free += Size;

  ptmmgr->handle_to_offset_and_size.erase (Handle);
  trie_del (&ptmmgr->offset_to_handle, Offset, fUserNone);

  PMemSize_t PSizeL = trie_get (ptmmgr->free_segment_end, Offset);
  PMemSize_t PSizeR = trie_get (ptmmgr->free_segment_start, Offset + Size);

  if (Offset + Size == ptmmgr->high_water)
    {
      ptmmgr->high_water = Offset - ((PSizeL == nullptr) ? 0 : *PSizeL);
    }

  if (PSizeL != nullptr && PSizeR != nullptr)
    {
      /* join left and right segment */

      Size_t SizeL = *PSizeL;
      Size_t SizeR = *PSizeR;

      assert (Offset >= SizeL);

      Offset_t OffsetL = Offset - SizeL;
      Offset_t OffsetR = Offset + Size;

      tmmgr_del_free_seg (ptmmgr, OffsetL, SizeL);
      tmmgr_del_free_seg (ptmmgr, OffsetR, SizeR);
      tmmgr_ins_free_seg (ptmmgr, OffsetL, SizeL + Size + SizeR);
    }
  else if (PSizeL != nullptr)
    {
      /* longer the left segment */

      MemSize_t SizeL = *PSizeL;

      assert (Offset >= SizeL);

      Offset_t OffsetL = Offset - SizeL;

      tmmgr_del_free_seg (ptmmgr, OffsetL, SizeL);
      tmmgr_ins_free_seg (ptmmgr, OffsetL, SizeL + Size);
    }
  else if (PSizeR != nullptr)
    {
      /* longer the right segment */

      MemSize_t SizeR = *PSizeR;

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

  ++ptmmgr->num_free;
  ptmmgr->sum_free += Size;

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
tmmgr_memused (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == nullptr)
    return 0;

  ptmmgr_t ptmmgr = static_cast<ptmmgr_t> (Tmmgr);

  return ptmmgr->mem_size - ptmmgr->mem_free;
}

MemSize_t
tmmgr_minfree (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == nullptr)
    return 0;

  ptmmgr_t ptmmgr = static_cast<ptmmgr_t> (Tmmgr);

  return ptmmgr->min_free;
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

Count_t
tmmgr_numalloc (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == nullptr)
    return 0;

  ptmmgr_t ptmmgr = static_cast<ptmmgr_t> (Tmmgr);

  return ptmmgr->num_alloc;
}

Count_t
tmmgr_numfree (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == nullptr)
    return 0;

  ptmmgr_t ptmmgr = static_cast<ptmmgr_t> (Tmmgr);

  return ptmmgr->num_free;
}

MemSize_t
tmmgr_sumalloc (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == nullptr)
    return 0;

  ptmmgr_t ptmmgr = static_cast<ptmmgr_t> (Tmmgr);

  return ptmmgr->sum_alloc;
}

MemSize_t
tmmgr_sumfree (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == nullptr)
    return 0;

  ptmmgr_t ptmmgr = static_cast<ptmmgr_t> (Tmmgr);

  return ptmmgr->sum_free;
}

static void
fHeap ( const Offset_t Offset
      , const PValue_t FHG_UTIL_UNUSED
                       (PVal, "Size (for traversal of free_segment_start) and"
                              "Handle (for traversal of offset_to_handle)"
                              "are not stored in the offset heap"
                       )
      , void *PDat
      )
{
  static_cast<gspc::mmgr::heap*> (PDat)->insert (Offset);
}

void
tmmgr_defrag (PTmmgr_t PTmmgr, const fMemmove_t fMemmove,
              const PMemSize_t PFreeSizeWanted, void *PDat)
{
  if (PTmmgr == nullptr || *(ptmmgr_t *) PTmmgr == nullptr)
    return;

  ptmmgr_t ptmmgr = *(ptmmgr_t *) PTmmgr;

  if (PFreeSizeWanted != nullptr && *PFreeSizeWanted > tmmgr_memfree (ptmmgr))
    return;

  gspc::mmgr::heap HeapOffFree;
  gspc::mmgr::heap HeapOffUsed;

  trie_work (ptmmgr->free_segment_start, fHeap, &HeapOffFree);
  trie_work (ptmmgr->offset_to_handle, fHeap, &HeapOffUsed);

  while (HeapOffFree.size() > 0 && HeapOffUsed.size() > 0)
    {
      Offset_t OffsetFree {HeapOffFree.min()};
      Offset_t OffsetUsed {HeapOffUsed.min()};

      while (HeapOffUsed.size() > 0 && OffsetUsed < OffsetFree)
        {
          HeapOffUsed.delete_min();

          if (HeapOffUsed.size() > 0)
            OffsetUsed = HeapOffUsed.min();
        }

      if (HeapOffUsed.size() > 0)
        {
          if (PFreeSizeWanted != nullptr
              && OffsetFree + *PFreeSizeWanted <= OffsetUsed)
            goto HAVE_ENOUGH;

          PHandle_t PHandle = trie_get (ptmmgr->offset_to_handle, OffsetUsed);

          if (PHandle == nullptr)
            TMMGR_ERROR_STRANGE;

          Handle_t Handle = *PHandle;
          MemSize_t SizeUsed;

          if (tmmgr_offset_size (ptmmgr, Handle, nullptr, &SizeUsed) !=
              RET_SUCCESS)
            TMMGR_ERROR_STRANGE;

          if (fMemmove != nullptr)
            fMemmove (OffsetFree, OffsetUsed, SizeUsed, PDat);

          tmmgr_del (ptmmgr, Handle, OffsetUsed, SizeUsed);
          tmmgr_ins (ptmmgr, Handle, OffsetFree, SizeUsed);

          HeapOffFree.delete_min();
          HeapOffUsed.delete_min();

          if (HeapOffFree.size() == 0
              || HeapOffFree.min() != OffsetFree + SizeUsed)
            HeapOffFree.insert (OffsetFree + SizeUsed);
        }
    }

HAVE_ENOUGH:
  return;
}
