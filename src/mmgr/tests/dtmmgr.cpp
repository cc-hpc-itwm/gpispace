#define BOOST_TEST_MODULE mmgr_dtmmgr
#include <boost/test/unit_test.hpp>

#include <mmgr/dtmmgr.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_AUTO_TEST_CASE (dtmmgr)
{
  gspc::vmem::dtmmgr dtmmgr (45, 2);

  BOOST_REQUIRE_EQUAL (dtmmgr.memfree(), 44);

  BOOST_REQUIRE_EQUAL (dtmmgr.numhandle (gspc::vmem::dtmmgr::ARENA_UP), 0);

  BOOST_REQUIRE_EQUAL (dtmmgr.numhandle (gspc::vmem::dtmmgr::ARENA_DOWN), 0);

  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (0, gspc::vmem::dtmmgr::ARENA_UP, 1), gspc::vmem::tmmgr::ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (1, gspc::vmem::dtmmgr::ARENA_DOWN, 1), gspc::vmem::tmmgr::ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (2, gspc::vmem::dtmmgr::ARENA_UP, 1), gspc::vmem::tmmgr::ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (3, gspc::vmem::dtmmgr::ARENA_DOWN, 1), gspc::vmem::tmmgr::ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (4, gspc::vmem::dtmmgr::ARENA_UP, 1), gspc::vmem::tmmgr::ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (5, gspc::vmem::dtmmgr::ARENA_DOWN, 1), gspc::vmem::tmmgr::ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (6, gspc::vmem::dtmmgr::ARENA_UP, 1), gspc::vmem::tmmgr::ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (7, gspc::vmem::dtmmgr::ARENA_DOWN, 1), gspc::vmem::tmmgr::ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (8, gspc::vmem::dtmmgr::ARENA_UP, 1), gspc::vmem::tmmgr::ALLOC_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (9, gspc::vmem::dtmmgr::ARENA_DOWN, 1), gspc::vmem::tmmgr::ALLOC_SUCCESS);

  {
    gspc::vmem::Offset_t Offset (-1);

    dtmmgr.offset_size (0, gspc::vmem::dtmmgr::ARENA_UP, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 0);

    dtmmgr.offset_size (2, gspc::vmem::dtmmgr::ARENA_UP, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 2);

    dtmmgr.offset_size (4, gspc::vmem::dtmmgr::ARENA_UP, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 4);

    dtmmgr.offset_size (6, gspc::vmem::dtmmgr::ARENA_UP, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 6);

    dtmmgr.offset_size (8, gspc::vmem::dtmmgr::ARENA_UP, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 8);

    dtmmgr.offset_size (1, gspc::vmem::dtmmgr::ARENA_DOWN, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 42);

    dtmmgr.offset_size (3, gspc::vmem::dtmmgr::ARENA_DOWN, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 40);

    dtmmgr.offset_size (5, gspc::vmem::dtmmgr::ARENA_DOWN, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 38);

    dtmmgr.offset_size (7, gspc::vmem::dtmmgr::ARENA_DOWN, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 36);

    dtmmgr.offset_size (9, gspc::vmem::dtmmgr::ARENA_DOWN, &Offset, nullptr);

    BOOST_REQUIRE_EQUAL (Offset, 34);
  }

  BOOST_REQUIRE_EQUAL (dtmmgr.free (2, gspc::vmem::dtmmgr::ARENA_UP), gspc::vmem::tmmgr::RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr.free (6, gspc::vmem::dtmmgr::ARENA_UP), gspc::vmem::tmmgr::RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr.free (3, gspc::vmem::dtmmgr::ARENA_DOWN), gspc::vmem::tmmgr::RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr.free (7, gspc::vmem::dtmmgr::ARENA_DOWN), gspc::vmem::tmmgr::RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (dtmmgr.free (8, gspc::vmem::dtmmgr::ARENA_UP), gspc::vmem::tmmgr::RET_SUCCESS);
}
