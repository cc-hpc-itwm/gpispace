#include <boost/test/unit_test.hpp>

#include <mmgr/dtmmgr.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_AUTO_TEST_CASE (dtmmgr)
{
  gspc::vmem::dtmmgr dtmmgr (45, 2);

  BOOST_REQUIRE_EQUAL (dtmmgr.memfree(), 44);

  BOOST_REQUIRE_EQUAL (dtmmgr.numhandle (gspc::vmem::dtmmgr::ARENA_UP), 0);

  BOOST_REQUIRE_EQUAL (dtmmgr.numhandle (gspc::vmem::dtmmgr::ARENA_DOWN), 0);

  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (0, gspc::vmem::dtmmgr::ARENA_UP, 1).first, 0);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (1, gspc::vmem::dtmmgr::ARENA_DOWN, 1).first, 42);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (2, gspc::vmem::dtmmgr::ARENA_UP, 1).first, 2);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (3, gspc::vmem::dtmmgr::ARENA_DOWN, 1).first, 40);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (4, gspc::vmem::dtmmgr::ARENA_UP, 1).first, 4);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (5, gspc::vmem::dtmmgr::ARENA_DOWN, 1).first, 38);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (6, gspc::vmem::dtmmgr::ARENA_UP, 1).first, 6);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (7, gspc::vmem::dtmmgr::ARENA_DOWN, 1).first, 36);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (8, gspc::vmem::dtmmgr::ARENA_UP, 1).first, 8);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (9, gspc::vmem::dtmmgr::ARENA_DOWN, 1).first, 34);

  dtmmgr.free (2, gspc::vmem::dtmmgr::ARENA_UP);
  dtmmgr.free (6, gspc::vmem::dtmmgr::ARENA_UP);
  dtmmgr.free (3, gspc::vmem::dtmmgr::ARENA_DOWN);
  dtmmgr.free (7, gspc::vmem::dtmmgr::ARENA_DOWN);
  dtmmgr.free (8, gspc::vmem::dtmmgr::ARENA_UP);
}
