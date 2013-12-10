
#include <mmgr/tmmgr.h>

#include <mmgr/ostab.h>
#include <mmgr/trie.h>
#include <mmgr/fseg.h>
#include <mmgr/heap.h>

#include <mmgr/unused.h>

#include <assert.h>

typedef struct
{
  OStab_t handle_to_offset_and_size;
  TrieMap_t offset_to_handle;
  FSeg_t free_offset_by_size;
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

  fseg_del (&ptmmgr->free_offset_by_size, Size, OffsetStart,
            SMAP_DEL_DEFAULT);
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

      fseg_ins (&ptmmgr->free_offset_by_size, Size, OffsetStart);
      *trie_ins (&ptmmgr->free_segment_start, OffsetStart, NULL) = Size;
      *trie_ins (&ptmmgr->free_segment_end, OffsetEnd, NULL) = Size;
    }
}

MemSize_t
tmmgr_init (PTmmgr_t PTmmgr, const MemSize_t MemSize, const Align_t Align)
{
  if (PTmmgr == NULL || *(ptmmgr_t *) PTmmgr != NULL)
    return 0;

  *(ptmmgr_t *) PTmmgr = malloc (sizeof (tmmgr_t));

  ptmmgr_t ptmmgr = *(ptmmgr_t *) PTmmgr;

  if (ptmmgr == NULL)
    TMMGR_ERROR_MALLOC_FAILED;

  ptmmgr->handle_to_offset_and_size = (OStab_t) NULL;
  ptmmgr->offset_to_handle = (TrieMap_t) NULL;
  ptmmgr->free_offset_by_size = (FSeg_t) NULL;
  ptmmgr->free_segment_start = (TrieMap_t) NULL;
  ptmmgr->free_segment_end = (TrieMap_t) NULL;

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
  if (PTmmgr == NULL || *(ptmmgr_t *) PTmmgr == NULL)
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

  if (PFreeSize != NULL)
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
  if (PTmmgr == NULL || *(ptmmgr_t *) PTmmgr == NULL)
    return 0;

  ptmmgr_t ptmmgr = *(ptmmgr_t *) PTmmgr;
  MemSize_t Bytes = sizeof (tmmgr_t);

  Bytes += ostab_free (&ptmmgr->handle_to_offset_and_size);
  Bytes += trie_free (&ptmmgr->offset_to_handle, fUserNone);
  Bytes += fseg_free (&ptmmgr->free_offset_by_size);
  Bytes += trie_free (&ptmmgr->free_segment_start, fUserNone);
  Bytes += trie_free (&ptmmgr->free_segment_end, fUserNone);

  free (ptmmgr);

  *(ptmmgr_t *) PTmmgr = NULL;

  return Bytes;
}

HandleReturn_t
tmmgr_offset_size (const Tmmgr_t Tmmgr, const Handle_t Handle,
                   POffset_t POffset, PMemSize_t PMemSize)
{
  if (Tmmgr == NULL)
    return RET_FAILURE;

  ptmmgr_t ptmmgr = Tmmgr;

  Bool_t was_there =
    ostab_get (ptmmgr->handle_to_offset_and_size, Handle, POffset, PMemSize);

  if (was_there == False)
    return RET_HANDLE_UNKNOWN;

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

  assert (PSizeStart != NULL);
  assert (Size <= *PSizeStart);

#ifndef NDEBUG
  PMemSize_t PSizeEnd =
#endif
    trie_get (ptmmgr->free_segment_end, Offset + (*PSizeStart));

  assert (PSizeEnd != NULL);
  assert (*PSizeStart == *PSizeEnd);

  Size_t OldFreeSize = *PSizeStart;
  Size_t NewFreeSize = *PSizeStart - Size;

  tmmgr_del_free_seg (ptmmgr, Offset, OldFreeSize);
  tmmgr_ins_free_seg (ptmmgr, Offset + Size, NewFreeSize);

  ostab_ins (&ptmmgr->handle_to_offset_and_size, Handle, Offset, Size);
  *trie_ins (&ptmmgr->offset_to_handle, Offset, NULL) = Handle;
}

AllocReturn_t
tmmgr_alloc (PTmmgr_t PTmmgr, const Handle_t Handle,
             const MemSize_t SizeUnaligned)
{
  if (PTmmgr == NULL || *(ptmmgr_t *) PTmmgr == NULL)
    return ALLOC_FAILURE;

  ptmmgr_t ptmmgr = *(ptmmgr_t *) PTmmgr;

  const MemSize_t Size = alignUp (SizeUnaligned, ptmmgr->align);

  if (ptmmgr->mem_free < Size)
    return ALLOC_INSUFFICIENT_MEMORY;

  if (tmmgr_offset_size (ptmmgr, Handle, NULL, NULL) == RET_SUCCESS)
    return ALLOC_DUPLICATE_HANDLE;

  MemSize_t size = Size;

  POffset_t POffset =
    fseg_get_atleast_minimal (ptmmgr->free_offset_by_size, &size);

  if (POffset == NULL)
    return ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY;

  tmmgr_ins (ptmmgr, Handle, *POffset, Size);

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

  ostab_del (&ptmmgr->handle_to_offset_and_size, Handle);
  trie_del (&ptmmgr->offset_to_handle, Offset, fUserNone);

  PMemSize_t PSizeL = trie_get (ptmmgr->free_segment_end, Offset);
  PMemSize_t PSizeR = trie_get (ptmmgr->free_segment_start, Offset + Size);

  if (Offset + Size == ptmmgr->high_water)
    {
      ptmmgr->high_water = Offset - ((PSizeL == NULL) ? 0 : *PSizeL);
    }

  if (PSizeL != NULL && PSizeR != NULL)
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
  else if (PSizeL != NULL)
    {
      /* longer the left segment */

      MemSize_t SizeL = *PSizeL;

      assert (Offset >= SizeL);

      Offset_t OffsetL = Offset - SizeL;

      tmmgr_del_free_seg (ptmmgr, OffsetL, SizeL);
      tmmgr_ins_free_seg (ptmmgr, OffsetL, SizeL + Size);
    }
  else if (PSizeR != NULL)
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
  if (PTmmgr == NULL || *(ptmmgr_t *) PTmmgr == NULL)
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
  if (Tmmgr == NULL)
    return 0;

  ptmmgr_t ptmmgr = Tmmgr;

  return ptmmgr->mem_size;
}

MemSize_t
tmmgr_memfree (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == NULL)
    return 0;

  ptmmgr_t ptmmgr = Tmmgr;

  return ptmmgr->mem_free;
}

MemSize_t
tmmgr_memused (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == NULL)
    return 0;

  ptmmgr_t ptmmgr = Tmmgr;

  return ptmmgr->mem_size - ptmmgr->mem_free;
}

MemSize_t
tmmgr_minfree (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == NULL)
    return 0;

  ptmmgr_t ptmmgr = Tmmgr;

  return ptmmgr->min_free;
}

MemSize_t
tmmgr_highwater (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == NULL)
    return 0;

  ptmmgr_t ptmmgr = Tmmgr;

  return ptmmgr->high_water;
}

Count_t
tmmgr_numhandle (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == NULL)
    return 0;

  ptmmgr_t ptmmgr = Tmmgr;

  return ostab_size (ptmmgr->handle_to_offset_and_size);
}

Count_t
tmmgr_numalloc (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == NULL)
    return 0;

  ptmmgr_t ptmmgr = Tmmgr;

  return ptmmgr->num_alloc;
}

Count_t
tmmgr_numfree (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == NULL)
    return 0;

  ptmmgr_t ptmmgr = Tmmgr;

  return ptmmgr->num_free;
}

MemSize_t
tmmgr_sumalloc (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == NULL)
    return 0;

  ptmmgr_t ptmmgr = Tmmgr;

  return ptmmgr->sum_alloc;
}

MemSize_t
tmmgr_sumfree (const Tmmgr_t Tmmgr)
{
  if (Tmmgr == NULL)
    return 0;

  ptmmgr_t ptmmgr = Tmmgr;

  return ptmmgr->sum_free;
}

static void
fHeap (const Offset_t Offset, const PValue_t UNUSED (PVal), void *PDat)
{
  heap_ins (PDat, Offset);
}

void
tmmgr_defrag (PTmmgr_t PTmmgr, const fMemmove_t fMemmove,
              const PMemSize_t PFreeSizeWanted, void *PDat)
{
  if (PTmmgr == NULL || *(ptmmgr_t *) PTmmgr == NULL)
    return;

  ptmmgr_t ptmmgr = *(ptmmgr_t *) PTmmgr;

  if (PFreeSizeWanted != NULL && *PFreeSizeWanted > tmmgr_memfree (ptmmgr))
    return;

  Heap_t HeapOffFree = (Heap_t) NULL;
  Heap_t HeapOffUsed = (Heap_t) NULL;

  trie_work (ptmmgr->free_segment_start, fHeap, &HeapOffFree);
  trie_work (ptmmgr->offset_to_handle, fHeap, &HeapOffUsed);

  while (heap_size (HeapOffFree) > 0 && heap_size (HeapOffUsed) > 0)
    {
      Offset_t OffsetFree = heap_min (HeapOffFree);
      Offset_t OffsetUsed = heap_min (HeapOffUsed);

      while (heap_size (HeapOffUsed) > 0 && OffsetUsed < OffsetFree)
        {
          heap_delmin (&HeapOffUsed);

          if (heap_size (HeapOffUsed) > 0)
            OffsetUsed = heap_min (HeapOffUsed);
        }

      if (heap_size (HeapOffUsed) > 0)
        {
          if (PFreeSizeWanted != NULL
              && OffsetFree + *PFreeSizeWanted <= OffsetUsed)
            goto HAVE_ENOUGH;

          PHandle_t PHandle = trie_get (ptmmgr->offset_to_handle, OffsetUsed);

          if (PHandle == NULL)
            TMMGR_ERROR_STRANGE;

          Handle_t Handle = *PHandle;
          MemSize_t SizeUsed;

          if (tmmgr_offset_size (ptmmgr, Handle, NULL, &SizeUsed) !=
              RET_SUCCESS)
            TMMGR_ERROR_STRANGE;

          if (fMemmove != NULL)
            fMemmove (OffsetFree, OffsetUsed, SizeUsed, PDat);

          tmmgr_del (ptmmgr, Handle, OffsetUsed, SizeUsed);
          tmmgr_ins (ptmmgr, Handle, OffsetFree, SizeUsed);

          heap_delmin (&HeapOffFree);
          heap_delmin (&HeapOffUsed);

          if (heap_size (HeapOffFree) == 0
              || heap_min (HeapOffFree) != OffsetFree + SizeUsed)
            heap_ins (&HeapOffFree, OffsetFree + SizeUsed);
        }
    }

HAVE_ENOUGH:
  heap_free (&HeapOffFree);
  heap_free (&HeapOffUsed);
}

static void
fPrintOStab (const Handle_t Handle, const Offset_t Offset,
             const MemSize_t Size, void UNUSED (*Pdat))
{
  printf (" " FMT_Handle_t "-(" FMT_Offset_t "," FMT_MemSize_t ")", Handle,
          Offset, Size);
}

static void
fPrintTrie (const Offset_t Offset, const PHandle_t PHandle,
            void UNUSED (*Pdat))
{
  printf (" " FMT_Offset_t "-" FMT_Offset_t, Offset, *PHandle);
}

static void
fPrintTrieHandle (const Handle_t Handle, const PValue_t UNUSED (PVal),
                  void UNUSED (*Pdat))
{
  printf (" " FMT_Handle_t, Handle);
}

static void
fPrintFSeg (const Handle_t Handle, const Value_t Value, void UNUSED (*Pdat))
{
  printf (" " FMT_Handle_t "-[", Handle);

  trie_work ((TrieMap_t) Value, &fPrintTrieHandle, NULL);

  printf ("]");
}

void
tmmgr_info (const Tmmgr_t Tmmgr, const char *msg)
{
  if (Tmmgr == NULL)
    return;

  ptmmgr_t ptmmgr = Tmmgr;

  Count_t nhandle = tmmgr_numhandle (Tmmgr);
  Count_t nalloc = tmmgr_numalloc (Tmmgr);
  Count_t nfree = tmmgr_numfree (Tmmgr);
  MemSize_t salloc = tmmgr_sumalloc (Tmmgr);
  MemSize_t sfree = tmmgr_sumfree (Tmmgr);
  MemSize_t mfree = tmmgr_memfree (Tmmgr);
  MemSize_t mused = tmmgr_memused (Tmmgr);
  MemSize_t mmin = tmmgr_minfree (Tmmgr);
  MemSize_t hwater = tmmgr_highwater (Tmmgr);

  printf ("*****%s"
          " used = " FMT_MemSize_t
          ", free = " FMT_MemSize_t
          ", size = " FMT_MemSize_t
          ", numhandle = " FMT_Count_t
          ", numalloc = " FMT_Count_t
          ", numfree = " FMT_Count_t
          ", sumalloc = " FMT_MemSize_t
          ", sumfree = " FMT_MemSize_t
          ", memfree = " FMT_MemSize_t
          ", memused = " FMT_MemSize_t
          ", minfree = " FMT_MemSize_t
          ", highwater = " FMT_MemSize_t
          "\n", (msg == NULL ? "" : msg),
          ptmmgr->mem_size - ptmmgr->mem_free, ptmmgr->mem_free,
          ptmmgr->mem_size, nhandle, nalloc, nfree, salloc, sfree, mfree,
          mused, mmin, hwater);
}

void
tmmgr_status (const Tmmgr_t Tmmgr, const char *msg)
{
  if (Tmmgr == NULL)
    return;

  ptmmgr_t ptmmgr = Tmmgr;

  tmmgr_info (ptmmgr, msg);

  printf ("allocs (handle-(offset,size)) =\n[");

  ostab_work (ptmmgr->handle_to_offset_and_size, &fPrintOStab, NULL);

  printf ("]\n");

  printf ("allocs (offset-handle) =\n[");

  trie_work (ptmmgr->offset_to_handle, &fPrintTrie, NULL);

  printf ("]\n");

  printf ("free_seg_by_size (size-[offset]) =\n[");

  smap_work (ptmmgr->free_offset_by_size, fPrintFSeg, NULL);

  printf ("]\n");

  printf ("free_seg_start (offset-size) =\n[");

  trie_work (ptmmgr->free_segment_start, &fPrintTrie, NULL);

  printf ("]\n");

  printf ("free_seg_end (offset-size) =\n[");

  trie_work (ptmmgr->free_segment_end, &fPrintTrie, NULL);

  printf ("]\n");
}
