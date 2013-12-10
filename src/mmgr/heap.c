
#include <mmgr/heap.h>

#include <mmgr/bool.h>

#include <stdlib.h>
#include <string.h>

typedef struct
{
  POffset_t arr;
  Size_t pos;
  Size_t size;
} heap_t, *pheap_t;

void
heap_out (const Heap_t Heap)
{
  printf ("[");

  if (Heap != NULL)
    {
      const pheap_t pheap = Heap;

      printf ("|" FMT_Size_t " " FMT_Size_t "|", pheap->pos, pheap->size);

      for (Size_t i = 0; i < pheap->pos; ++i)
        {
          printf (" " FMT_Offset_t, pheap->arr[i]);
        }
    }

  printf ("]\n");
}

Size_t
heap_size (const Heap_t Heap)
{
  if (Heap == NULL)
    return 0;

  const pheap_t pheap = Heap;

  return pheap->pos;
}

Offset_t
heap_min (const Heap_t Heap)
{
  if (heap_size (Heap) == 0)
    HEAP_ERROR_EMPTY;

  const pheap_t pheap = Heap;

  return pheap->arr[0];
}

static inline pheap_t
heap_mk (Size_t size)
{
  if (size == 0)
    return NULL;

  const pheap_t pheap = malloc (sizeof (heap_t));

  if (pheap == NULL)
    HEAP_ERROR_MALLOC_FAILED;

  pheap->arr = malloc (size * sizeof (Offset_t));

  if (pheap->arr == NULL)
    HEAP_ERROR_MALLOC_FAILED;

  pheap->pos = 0;
  pheap->size = size;

  return pheap;
}

static inline void
heap_copy (pheap_t dest, pheap_t src, Size_t size)
{
  if (dest == NULL || src == NULL)
    return;

  memcpy (dest->arr, src->arr, size * sizeof (Offset_t));

  dest->pos = src->pos;
}

static inline Size_t
parent (Size_t Node)
{
  return (Node == 0) ? 0 : (Node - 1) / 2;
}

static inline Size_t
childL (Size_t Node)
{
  return (2 * Node + 1);
}

static inline Size_t
childR (Size_t Node)
{
  return (2 * Node + 2);
}

static inline void
swap (pheap_t pheap, Size_t x, Size_t y)
{
  Offset_t t = pheap->arr[x];
  pheap->arr[x] = pheap->arr[y];
  pheap->arr[y] = t;
}

void
heap_ins (PHeap_t PHeap, const Offset_t Offset)
{
  if (PHeap == NULL)
    return;

  if (*(pheap_t *) PHeap == NULL)
    {
      *(pheap_t *) PHeap = heap_mk (heap_initial_size);
    }

  {
    pheap_t pheap = *(pheap_t *) PHeap;

    if (pheap->pos == pheap->size)
      {
        pheap_t pheap_new = heap_mk (2 * pheap->size);

        heap_copy (pheap_new, pheap, pheap->size);

        heap_free (PHeap);

        *(pheap_t *) PHeap = pheap_new;
      }
  }

  pheap_t pheap = *(pheap_t *) PHeap;

  pheap->arr[pheap->pos] = Offset;

  Size_t Node = pheap->pos;
  Size_t Parent = parent (Node);

  while (Parent != Node && pheap->arr[Parent] > pheap->arr[Node])
    {
      swap (pheap, Parent, Node);

      Node = Parent;
      Parent = parent (Node);
    }

  ++pheap->pos;
}

static inline Bool_t
small (pheap_t pheap, Size_t Node, Size_t Child)
{
  return (Child >= pheap->pos) ? False : (pheap->arr[Child] <
                                          pheap->arr[Node] ? True : False);
}

void
heap_delmin (PHeap_t PHeap)
{
  if (PHeap == NULL || *(pheap_t *) PHeap == NULL)
    return;

  pheap_t pheap = *(pheap_t *) PHeap;

  --pheap->pos;

  pheap->arr[0] = pheap->arr[pheap->pos];

  Size_t Node = 0;

  Size_t ChildL = childL (Node);
  Size_t ChildR = childR (Node);

  Bool_t smallL = small (pheap, Node, ChildL);
  Bool_t smallR = small (pheap, Node, ChildR);

  while (smallL == True || smallR == True)
    {
      if (smallL == True && smallR == True)
        {
          if (pheap->arr[ChildL] > pheap->arr[ChildR])
            {
              smallL = False;
            }
        }

      if (smallL == True)
        {
          swap (pheap, Node, ChildL);

          Node = ChildL;
        }
      else
        {
          swap (pheap, Node, ChildR);

          Node = ChildR;
        }

      ChildL = childL (Node);
      ChildR = childR (Node);

      smallL = small (pheap, Node, ChildL);
      smallR = small (pheap, Node, ChildR);
    }

  if (pheap->pos < pheap->size / 2 && pheap->size / 2 >= heap_minimal_size)
    {
      pheap_t pheap_new = heap_mk (pheap->size / 2);

      heap_copy (pheap_new, pheap, pheap_new->size);

      heap_free (PHeap);

      *(pheap_t *) PHeap = pheap_new;
    }
}

Size_t
heap_free (PHeap_t PHeap)
{
  if (PHeap == NULL || *(pheap_t *) PHeap == NULL)
    return 0;

  pheap_t pheap = *(pheap_t *) PHeap;

  Size_t Bytes = pheap->size * sizeof (Offset_t) + sizeof (heap_t);

  free (pheap->arr);
  free (pheap);

  *(pheap_t *) PHeap = NULL;

  return Bytes;
}
