#include <stdlib.h>
#include <assert.h>

#include <mmgr/dtmmgr.h>

#include <initializer_list>

#define FOR_ARENA(id) for (Arena_t id : {ARENA_UP, ARENA_DOWN})

static const Arena_t Other[2] = { ARENA_DOWN, ARENA_UP };

typedef struct
{
  PTmmgr_t arena[2];
  MemSize_t mem_size;
} dtmmgr_t, *pdtmmgr_t;

void
dtmmgr_init (PDTmmgr_t PDTmmgr, const MemSize_t MemSize, const Align_t Align)
{
  if (PDTmmgr == nullptr || *(pdtmmgr_t *) PDTmmgr != nullptr)
    return;

  *(pdtmmgr_t *) PDTmmgr = static_cast<pdtmmgr_t> (malloc (sizeof (dtmmgr_t)));

  pdtmmgr_t pdtmmgr = *(pdtmmgr_t *) PDTmmgr;

  if (pdtmmgr == nullptr)
    DTMMGR_ERROR_MALLOC_FAILED;

  FOR_ARENA (Arena)
    {
      pdtmmgr->arena[Arena] = nullptr;

      tmmgr_init (pdtmmgr->arena + Arena, MemSize, Align);
    }

  // get the real size after alignment
  pdtmmgr->mem_size = tmmgr_memsize (pdtmmgr->arena[ARENA_UP]);
}

MemSize_t
dtmmgr_finalize (PDTmmgr_t PDTmmgr)
{
  if (PDTmmgr == nullptr || *(pdtmmgr_t *) PDTmmgr == nullptr)
    return 0;

  pdtmmgr_t pdtmmgr = *(pdtmmgr_t *) PDTmmgr;

  MemSize_t Bytes = sizeof (dtmmgr_t);

  FOR_ARENA (Arena) Bytes += tmmgr_finalize (pdtmmgr->arena + Arena);

  free (pdtmmgr);

  *(pdtmmgr_t *) PDTmmgr = nullptr;

  return Bytes;
}

AllocReturn_t
dtmmgr_alloc (PDTmmgr_t PDTmmgr, const Handle_t Handle, const Arena_t Arena,
              const MemSize_t Size)
{
  if (PDTmmgr == nullptr || *(pdtmmgr_t *) PDTmmgr == nullptr)
    return ALLOC_FAILURE;

  pdtmmgr_t pdtmmgr = *(pdtmmgr_t *) PDTmmgr;

  AllocReturn_t AllocReturn =
    tmmgr_alloc (pdtmmgr->arena + Arena, Handle, Size);

  if (AllocReturn == ALLOC_SUCCESS)
    tmmgr_resize (pdtmmgr->arena + Other[Arena],
                  pdtmmgr->mem_size -
                  tmmgr_highwater (pdtmmgr->arena[Arena]));

  return AllocReturn;
}

HandleReturn_t
dtmmgr_free (PDTmmgr_t PDTmmgr, const Handle_t Handle, const Arena_t Arena)
{
  if (PDTmmgr == nullptr || *(pdtmmgr_t *) PDTmmgr == nullptr)
    return RET_FAILURE;

  pdtmmgr_t pdtmmgr = *(pdtmmgr_t *) PDTmmgr;

  HandleReturn_t HandleReturn = tmmgr_free (pdtmmgr->arena + Arena, Handle);

  if (HandleReturn == RET_SUCCESS)
    tmmgr_resize (pdtmmgr->arena + Other[Arena],
                  pdtmmgr->mem_size -
                  tmmgr_highwater (pdtmmgr->arena[Arena]));

  return HandleReturn;
}

HandleReturn_t
dtmmgr_offset_size (const DTmmgr_t DTmmgr, const Handle_t Handle,
                    const Arena_t Arena, POffset_t POffset,
                    PMemSize_t PMemSize)
{
  if (DTmmgr == nullptr)
    return RET_FAILURE;

  pdtmmgr_t pdtmmgr = static_cast<pdtmmgr_t> (DTmmgr);

  MemSize_t Size = 0;

  HandleReturn_t HandleReturn =
    tmmgr_offset_size (pdtmmgr->arena[Arena], Handle, POffset, &Size);

  if (HandleReturn == RET_SUCCESS)
    {
      // invert for the local arena
      if (Arena == ARENA_DOWN && POffset != nullptr)
        {
          assert (pdtmmgr->mem_size >= *POffset + Size);

          *POffset = pdtmmgr->mem_size - (*POffset + Size);
        }

      if (PMemSize != nullptr)
        *PMemSize = Size;
    }

  return HandleReturn;
}

MemSize_t
dtmmgr_memfree (const DTmmgr_t DTmmgr)
{
  if (DTmmgr == nullptr)
    return 0;

  pdtmmgr_t pdtmmgr = static_cast<pdtmmgr_t> (DTmmgr);

  MemSize_t FreeGlobal = tmmgr_memfree (pdtmmgr->arena[ARENA_UP]);
  MemSize_t FreeLocal = tmmgr_memfree (pdtmmgr->arena[ARENA_DOWN]);

  return (FreeGlobal > FreeLocal) ? FreeGlobal : FreeLocal;
}

Count_t
dtmmgr_numhandle (const DTmmgr_t DTmmgr, const Arena_t Arena)
{
  if (DTmmgr == nullptr)
    return 0;

  pdtmmgr_t pdtmmgr = static_cast<pdtmmgr_t> (DTmmgr);

  return tmmgr_numhandle (pdtmmgr->arena[Arena]);
}
