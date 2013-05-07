
#include <mmgr/heap.h>

#include <stdio.h>
#include <stdlib.h>

#ifndef __APPLE__
// malloc.h is deprecated on OSX.
#include <malloc.h>
#else
// malloc_stats() is missing on OSX / FreeBSD / Solaris / ...
void malloc_stats() { }
#endif

int
main ()
{
  Heap_t h = NULL;

  heap_out (h);

  heap_ins (&h, 12);
  heap_out (h);
  heap_ins (&h, 13);
  heap_out (h);

  heap_delmin (&h);
  heap_out (h);

  heap_delmin (&h);
  heap_out (h);

  heap_ins (&h, 10);
  heap_out (h);
  heap_ins (&h, 11);
  heap_out (h);
  heap_ins (&h, 12);
  heap_out (h);
  heap_ins (&h, 9);
  heap_out (h);
  heap_ins (&h, 8);
  heap_out (h);
  heap_ins (&h, 7);
  heap_out (h);

  while (heap_size (h) > 0)
    {
      printf (" " FMT_Offset_t " ", heap_min (h));

      heap_delmin (&h);
      heap_out (h);
    }

  heap_ins (&h, 10);
  heap_out (h);
  heap_ins (&h, 11);
  heap_out (h);
  heap_ins (&h, 12);
  heap_out (h);
  heap_ins (&h, 9);
  heap_out (h);
  heap_ins (&h, 8);
  heap_out (h);
  heap_ins (&h, 7);
  heap_out (h);

  heap_ins (&h, 10);
  heap_out (h);
  heap_ins (&h, 11);
  heap_out (h);
  heap_ins (&h, 12);
  heap_out (h);
  heap_ins (&h, 9);
  heap_out (h);
  heap_ins (&h, 8);
  heap_out (h);
  heap_ins (&h, 7);
  heap_out (h);

  while (heap_size (h) > 0)
    {
      printf (" " FMT_Offset_t " ", heap_min (h));

      heap_delmin (&h);
      heap_out (h);
    }

  printf ("\n");

  printf ("free = " FMT_Size_t "\n", heap_free (&h));

  malloc_stats ();

  return EXIT_SUCCESS;
}
