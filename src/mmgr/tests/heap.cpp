#define BOOST_TEST_MODULE mmgr_heap
#include <boost/test/unit_test.hpp>

#include <mmgr/heap.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

BOOST_AUTO_TEST_CASE (heap)
{
  gspc::mmgr::heap h;

  BOOST_REQUIRE_EQUAL (h.size(), 0);

  h.insert (12);

  BOOST_REQUIRE_EQUAL (h.size(), 1);
  BOOST_REQUIRE_EQUAL (h.min(), 12);

  h.insert (13);

  BOOST_REQUIRE_EQUAL (h.size(), 2);
  BOOST_REQUIRE_EQUAL (h.min(), 12);
  h.delete_min();

  BOOST_REQUIRE_EQUAL (h.size(), 1);
  BOOST_REQUIRE_EQUAL (h.min(), 13);
  h.delete_min();

  BOOST_REQUIRE_EQUAL (h.size(), 0);

  h.insert (10);
  h.insert (11);
  h.insert (12);
  h.insert (9);
  h.insert (8);
  h.insert (7);

  BOOST_REQUIRE_EQUAL (h.min(), 7);
  h.delete_min();

  BOOST_REQUIRE_EQUAL (h.min(), 8);
  h.delete_min();

  BOOST_REQUIRE_EQUAL (h.min(), 9);
  h.delete_min();

  BOOST_REQUIRE_EQUAL (h.min(), 10);
  h.delete_min();

  BOOST_REQUIRE_EQUAL (h.min(), 11);
  h.delete_min();

  BOOST_REQUIRE_EQUAL (h.min(), 12);
  h.delete_min();

  BOOST_REQUIRE_EQUAL (h.size(), 0);

  h.insert (10);
  h.insert (11);
  h.insert (12);
  h.insert (9);
  h.insert (8);
  h.insert (7);
  h.insert (10);
  h.insert (11);
  h.insert (12);
  h.insert (9);
  h.insert (8);
  h.insert (7);

  BOOST_REQUIRE_EQUAL (h.min(), 7);
  h.delete_min();
  BOOST_REQUIRE_EQUAL (h.min(), 7);
  h.delete_min();

  BOOST_REQUIRE_EQUAL (h.min(), 8);
  h.delete_min();
  BOOST_REQUIRE_EQUAL (h.min(), 8);
  h.delete_min();

  BOOST_REQUIRE_EQUAL (h.min(), 9);
  h.delete_min();
  BOOST_REQUIRE_EQUAL (h.min(), 9);
  h.delete_min();

  BOOST_REQUIRE_EQUAL (h.min(), 10);
  h.delete_min();
  BOOST_REQUIRE_EQUAL (h.min(), 10);
  h.delete_min();

  BOOST_REQUIRE_EQUAL (h.min(), 11);
  h.delete_min();
  BOOST_REQUIRE_EQUAL (h.min(), 11);
  h.delete_min();

  BOOST_REQUIRE_EQUAL (h.min(), 12);
  h.delete_min();
  BOOST_REQUIRE_EQUAL (h.min(), 12);
  h.delete_min();

  BOOST_REQUIRE_EQUAL (h.size(), 0);
}

BOOST_AUTO_TEST_CASE (min_empty_throws_heap_empty)
{
  fhg::util::testing::require_exception
    ( []() { gspc::mmgr::heap().min(); }
    , gspc::mmgr::exception::heap_empty ("min")
    );
}

BOOST_AUTO_TEST_CASE (delete_min_empty_throws_heap_empty)
{
  fhg::util::testing::require_exception
    ( []() { gspc::mmgr::heap().delete_min(); }
    , gspc::mmgr::exception::heap_empty ("delete_min")
    );
}
