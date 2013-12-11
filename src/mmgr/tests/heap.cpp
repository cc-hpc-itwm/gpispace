// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE expr_type_calculate
#include <boost/test/unit_test.hpp>

#include <mmgr/heap.h>

#include <stdio.h>
#include <stdlib.h>

BOOST_AUTO_TEST_CASE (heap)
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
}
