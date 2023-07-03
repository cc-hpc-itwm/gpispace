// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <iml/testing/random/AllocationHandle.hpp>
#include <iml/vmem/dtmmgr.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>

#include <vector>

BOOST_AUTO_TEST_CASE (dtmmgr_works)
{
  iml_client::vmem::dtmmgr dtmmgr (45, 2);

  BOOST_REQUIRE_EQUAL (dtmmgr.memfree(), 44);

  BOOST_REQUIRE_EQUAL (dtmmgr.numhandle (iml_client::vmem::dtmmgr::ARENA_UP), 0);

  BOOST_REQUIRE_EQUAL (dtmmgr.numhandle (iml_client::vmem::dtmmgr::ARENA_DOWN), 0);

  auto const handles
    ( fhg::util::testing::unique_randoms<std::vector<iml::AllocationHandle>>
        (10)
    );

  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (handles[0], iml_client::vmem::dtmmgr::ARENA_UP, 1).first, 0);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (handles[1], iml_client::vmem::dtmmgr::ARENA_DOWN, 1).first, 42);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (handles[2], iml_client::vmem::dtmmgr::ARENA_UP, 1).first, 2);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (handles[3], iml_client::vmem::dtmmgr::ARENA_DOWN, 1).first, 40);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (handles[4], iml_client::vmem::dtmmgr::ARENA_UP, 1).first, 4);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (handles[5], iml_client::vmem::dtmmgr::ARENA_DOWN, 1).first, 38);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (handles[6], iml_client::vmem::dtmmgr::ARENA_UP, 1).first, 6);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (handles[7], iml_client::vmem::dtmmgr::ARENA_DOWN, 1).first, 36);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (handles[8], iml_client::vmem::dtmmgr::ARENA_UP, 1).first, 8);
  BOOST_REQUIRE_EQUAL (dtmmgr.alloc (handles[9], iml_client::vmem::dtmmgr::ARENA_DOWN, 1).first, 34);

  dtmmgr.free (handles[2], iml_client::vmem::dtmmgr::ARENA_UP);
  dtmmgr.free (handles[6], iml_client::vmem::dtmmgr::ARENA_UP);
  dtmmgr.free (handles[3], iml_client::vmem::dtmmgr::ARENA_DOWN);
  dtmmgr.free (handles[7], iml_client::vmem::dtmmgr::ARENA_DOWN);
  dtmmgr.free (handles[8], iml_client::vmem::dtmmgr::ARENA_UP);
}
