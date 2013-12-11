// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE expr_type_calculate
#include <boost/test/unit_test.hpp>

#include <mmgr/dtmmgr.h>

#include <stdio.h>
#include <stdlib.h>

static const Arena_t Other[2] = { ARENA_DOWN, ARENA_UP };

namespace
{
  void fMemmove ( const OffsetDest_t OffsetDest, const OffsetSrc_t OffsetSrc
                , const MemSize_t Size, void *PDat)
  {
    printf ("CALLBACK-%lu: Moving " FMT_MemSize_t " Byte(s) from " FMT_Offset_t
           " to " FMT_Offset_t "\n", (*(unsigned long *) PDat)++, Size,
           OffsetSrc, OffsetDest);
  }
}

BOOST_AUTO_TEST_CASE (dtmmgr)
{
  DTmmgr_t dtmmgr = NULL;

  dtmmgr_init (&dtmmgr, 45, 2);

  BOOST_REQUIRE_EQUAL (dtmmgr_memsize (dtmmgr), 44);
  BOOST_REQUIRE_EQUAL (dtmmgr_memfree (dtmmgr), 44);
  BOOST_REQUIRE_EQUAL (dtmmgr_memused (dtmmgr), 0);

  BOOST_REQUIRE_EQUAL (dtmmgr_numalloc (dtmmgr, ARENA_UP), 0);
  BOOST_REQUIRE_EQUAL (dtmmgr_numfree (dtmmgr, ARENA_UP), 0);
  BOOST_REQUIRE_EQUAL (dtmmgr_numhandle (dtmmgr, ARENA_UP), 0);
  BOOST_REQUIRE_EQUAL (dtmmgr_sumalloc (dtmmgr, ARENA_UP), 0);
  BOOST_REQUIRE_EQUAL (dtmmgr_sumfree (dtmmgr, ARENA_UP), 0);

  BOOST_REQUIRE_EQUAL (dtmmgr_numalloc (dtmmgr, ARENA_DOWN), 0);
  BOOST_REQUIRE_EQUAL (dtmmgr_numfree (dtmmgr, ARENA_DOWN), 0);
  BOOST_REQUIRE_EQUAL (dtmmgr_numhandle (dtmmgr, ARENA_DOWN), 0);
  BOOST_REQUIRE_EQUAL (dtmmgr_sumalloc (dtmmgr, ARENA_DOWN), 0);
  BOOST_REQUIRE_EQUAL (dtmmgr_sumfree (dtmmgr, ARENA_DOWN), 0);

  BOOST_REQUIRE_EQUAL (dtmmgr_alloc (&dtmmgr, 0, ARENA_UP, 1), ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_alloc (&dtmmgr, 1, ARENA_DOWN, 1), ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_alloc (&dtmmgr, 2, ARENA_UP, 1), ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_alloc (&dtmmgr, 3, ARENA_DOWN, 1), ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_alloc (&dtmmgr, 4, ARENA_UP, 1), ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_alloc (&dtmmgr, 5, ARENA_DOWN, 1), ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_alloc (&dtmmgr, 6, ARENA_UP, 1), ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_alloc (&dtmmgr, 7, ARENA_DOWN, 1), ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_alloc (&dtmmgr, 8, ARENA_UP, 1), ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_alloc (&dtmmgr, 9, ARENA_DOWN, 1), ALLOC_SUCCESS);

  {
    Offset_t Offset = -1;

    dtmmgr_offset_size (dtmmgr, 0, ARENA_UP, &Offset, NULL);

    BOOST_REQUIRE_EQUAL (Offset, 0);

    dtmmgr_offset_size (dtmmgr, 2, ARENA_UP, &Offset, NULL);

    BOOST_REQUIRE_EQUAL (Offset, 2);

    dtmmgr_offset_size (dtmmgr, 4, ARENA_UP, &Offset, NULL);

    BOOST_REQUIRE_EQUAL (Offset, 4);

    dtmmgr_offset_size (dtmmgr, 6, ARENA_UP, &Offset, NULL);

    BOOST_REQUIRE_EQUAL (Offset, 6);

    dtmmgr_offset_size (dtmmgr, 8, ARENA_UP, &Offset, NULL);

    BOOST_REQUIRE_EQUAL (Offset, 8);

    dtmmgr_offset_size (dtmmgr, 1, ARENA_DOWN, &Offset, NULL);

    BOOST_REQUIRE_EQUAL (Offset, 42);

    dtmmgr_offset_size (dtmmgr, 3, ARENA_DOWN, &Offset, NULL);

    BOOST_REQUIRE_EQUAL (Offset, 40);

    dtmmgr_offset_size (dtmmgr, 5, ARENA_DOWN, &Offset, NULL);

    BOOST_REQUIRE_EQUAL (Offset, 38);

    dtmmgr_offset_size (dtmmgr, 7, ARENA_DOWN, &Offset, NULL);

    BOOST_REQUIRE_EQUAL (Offset, 36);

    dtmmgr_offset_size (dtmmgr, 9, ARENA_DOWN, &Offset, NULL);

    BOOST_REQUIRE_EQUAL (Offset, 34);
  }

  BOOST_REQUIRE_EQUAL (dtmmgr_free (&dtmmgr, 2, ARENA_UP), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_free (&dtmmgr, 6, ARENA_UP), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_free (&dtmmgr, 3, ARENA_DOWN), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_free (&dtmmgr, 7, ARENA_DOWN), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_free (&dtmmgr, 8, ARENA_UP), RET_SUCCESS);

  unsigned long callback_count = 0;

  dtmmgr_defrag (&dtmmgr, ARENA_UP, &fMemmove, NULL, &callback_count);
  dtmmgr_defrag (&dtmmgr, ARENA_DOWN, &fMemmove, NULL, &callback_count);

  {
    printf ("%p %p\n", &dtmmgr, dtmmgr);

    Size_t Bytes = dtmmgr_finalize (&dtmmgr);

    printf ("Bytes = " FMT_Size_t "\n", Bytes);
  }

  {
    printf ("%p %p\n", &dtmmgr, dtmmgr);

    Size_t Bytes = dtmmgr_finalize (&dtmmgr);

    printf ("Bytes = " FMT_Size_t "\n", Bytes);
  }
}
