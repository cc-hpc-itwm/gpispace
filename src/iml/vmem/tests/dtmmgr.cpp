#include <boost/test/unit_test.hpp>

#include <iml/vmem/dtmmgr.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_AUTO_TEST_CASE (dtmmgr)
{
  iml_client::vmem::dtmmgr dtmmgr (45, 2);

  BOOST_REQUIRE_EQUAL (dtmmgr.memfree(), 44);

  BOOST_REQUIRE_EQUAL (dtmmgr.numhandle (iml_client::vmem::dtmmgr::ARENA_UP), 0);

  BOOST_REQUIRE_EQUAL (dtmmgr.numhandle (iml_client::vmem::dtmmgr::ARENA_DOWN), 0);

  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (0, iml_client::vmem::dtmmgr::ARENA_UP, 1).first, 0);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (1, iml_client::vmem::dtmmgr::ARENA_DOWN, 1).first, 42);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (2, iml_client::vmem::dtmmgr::ARENA_UP, 1).first, 2);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (3, iml_client::vmem::dtmmgr::ARENA_DOWN, 1).first, 40);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (4, iml_client::vmem::dtmmgr::ARENA_UP, 1).first, 4);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (5, iml_client::vmem::dtmmgr::ARENA_DOWN, 1).first, 38);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (6, iml_client::vmem::dtmmgr::ARENA_UP, 1).first, 6);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (7, iml_client::vmem::dtmmgr::ARENA_DOWN, 1).first, 36);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (8, iml_client::vmem::dtmmgr::ARENA_UP, 1).first, 8);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (9, iml_client::vmem::dtmmgr::ARENA_DOWN, 1).first, 34);

  dtmmgr.free (2, iml_client::vmem::dtmmgr::ARENA_UP);
  dtmmgr.free (6, iml_client::vmem::dtmmgr::ARENA_UP);
  dtmmgr.free (3, iml_client::vmem::dtmmgr::ARENA_DOWN);
  dtmmgr.free (7, iml_client::vmem::dtmmgr::ARENA_DOWN);
  dtmmgr.free (8, iml_client::vmem::dtmmgr::ARENA_UP);
}
