// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE expr_type_calculate
#include <boost/test/unit_test.hpp>

#include <mmgr/heap.h>

BOOST_AUTO_TEST_CASE (heap)
{
  Heap_t h = NULL;

  BOOST_REQUIRE_EQUAL (heap_size (h), 0);

  heap_ins (&h, 12);

  BOOST_REQUIRE_EQUAL (heap_size (h), 1);
  BOOST_REQUIRE_EQUAL (heap_min (h), 12);

  heap_ins (&h, 13);

  BOOST_REQUIRE_EQUAL (heap_size (h), 2);
  BOOST_REQUIRE_EQUAL (heap_min (h), 12);
  heap_delmin (&h);

  BOOST_REQUIRE_EQUAL (heap_size (h), 1);
  BOOST_REQUIRE_EQUAL (heap_min (h), 13);
  heap_delmin (&h);

  BOOST_REQUIRE_EQUAL (heap_size (h), 0);

  heap_ins (&h, 10);
  heap_ins (&h, 11);
  heap_ins (&h, 12);
  heap_ins (&h, 9);
  heap_ins (&h, 8);
  heap_ins (&h, 7);

  BOOST_REQUIRE_EQUAL (heap_min (h), 7);
  heap_delmin (&h);

  BOOST_REQUIRE_EQUAL (heap_min (h), 8);
  heap_delmin (&h);

  BOOST_REQUIRE_EQUAL (heap_min (h), 9);
  heap_delmin (&h);

  BOOST_REQUIRE_EQUAL (heap_min (h), 10);
  heap_delmin (&h);

  BOOST_REQUIRE_EQUAL (heap_min (h), 11);
  heap_delmin (&h);

  BOOST_REQUIRE_EQUAL (heap_min (h), 12);
  heap_delmin (&h);

  BOOST_REQUIRE_EQUAL (heap_size (h), 0);

  heap_ins (&h, 10);
  heap_ins (&h, 11);
  heap_ins (&h, 12);
  heap_ins (&h, 9);
  heap_ins (&h, 8);
  heap_ins (&h, 7);
  heap_ins (&h, 10);
  heap_ins (&h, 11);
  heap_ins (&h, 12);
  heap_ins (&h, 9);
  heap_ins (&h, 8);
  heap_ins (&h, 7);

  BOOST_REQUIRE_EQUAL (heap_min (h), 7);
  heap_delmin (&h);
  BOOST_REQUIRE_EQUAL (heap_min (h), 7);
  heap_delmin (&h);

  BOOST_REQUIRE_EQUAL (heap_min (h), 8);
  heap_delmin (&h);
  BOOST_REQUIRE_EQUAL (heap_min (h), 8);
  heap_delmin (&h);

  BOOST_REQUIRE_EQUAL (heap_min (h), 9);
  heap_delmin (&h);
  BOOST_REQUIRE_EQUAL (heap_min (h), 9);
  heap_delmin (&h);

  BOOST_REQUIRE_EQUAL (heap_min (h), 10);
  heap_delmin (&h);
  BOOST_REQUIRE_EQUAL (heap_min (h), 10);
  heap_delmin (&h);

  BOOST_REQUIRE_EQUAL (heap_min (h), 11);
  heap_delmin (&h);
  BOOST_REQUIRE_EQUAL (heap_min (h), 11);
  heap_delmin (&h);

  BOOST_REQUIRE_EQUAL (heap_min (h), 12);
  heap_delmin (&h);
  BOOST_REQUIRE_EQUAL (heap_min (h), 12);
  heap_delmin (&h);

  BOOST_REQUIRE_EQUAL (heap_size (h), 0);

  BOOST_REQUIRE_EQUAL (heap_free (&h), 152);
}
