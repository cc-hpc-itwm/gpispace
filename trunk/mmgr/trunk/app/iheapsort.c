
#include <mmgr/heap.h>

#include <stdio.h>
#include <stdlib.h>

int
main ()
{
  Heap_t h = NULL;

  char buf[20];

  while (fgets (buf, 20, stdin) != NULL)
    {
      heap_ins (&h, atol (buf));
    }

  while (heap_size (h) > 0)
    {
      printf (FMT_Offset_t "\n", heap_min (h));

      heap_delmin (&h);
    }

  heap_free (&h);

  return EXIT_SUCCESS;
}
