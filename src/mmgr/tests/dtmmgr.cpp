// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE mmgr_dtmmgr
#include <boost/test/unit_test.hpp>

#include <mmgr/dtmmgr.h>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_AUTO_TEST_CASE (dtmmgr)
{
  DTmmgr_t dtmmgr = nullptr;

  dtmmgr_init (&dtmmgr, 45, 2);

  BOOST_REQUIRE_EQUAL (dtmmgr_memfree (dtmmgr), 44);

  BOOST_REQUIRE_EQUAL (dtmmgr_numhandle (dtmmgr, ARENA_UP), 0);

  BOOST_REQUIRE_EQUAL (dtmmgr_numhandle (dtmmgr, ARENA_DOWN), 0);

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

    dtmmgr_offset_size (dtmmgr, 0, ARENA_UP, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 0);

    dtmmgr_offset_size (dtmmgr, 2, ARENA_UP, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 2);

    dtmmgr_offset_size (dtmmgr, 4, ARENA_UP, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 4);

    dtmmgr_offset_size (dtmmgr, 6, ARENA_UP, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 6);

    dtmmgr_offset_size (dtmmgr, 8, ARENA_UP, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 8);

    dtmmgr_offset_size (dtmmgr, 1, ARENA_DOWN, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 42);

    dtmmgr_offset_size (dtmmgr, 3, ARENA_DOWN, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 40);

    dtmmgr_offset_size (dtmmgr, 5, ARENA_DOWN, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 38);

    dtmmgr_offset_size (dtmmgr, 7, ARENA_DOWN, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 36);

    dtmmgr_offset_size (dtmmgr, 9, ARENA_DOWN, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 34);
  }

  BOOST_REQUIRE_EQUAL (dtmmgr_free (&dtmmgr, 2, ARENA_UP), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_free (&dtmmgr, 6, ARENA_UP), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_free (&dtmmgr, 3, ARENA_DOWN), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_free (&dtmmgr, 7, ARENA_DOWN), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr_free (&dtmmgr, 8, ARENA_UP), RET_SUCCESS);

  BOOST_REQUIRE_GT (dtmmgr_finalize (&dtmmgr), 0);
  BOOST_REQUIRE_EQUAL (dtmmgr_finalize (&dtmmgr), 0);
}
