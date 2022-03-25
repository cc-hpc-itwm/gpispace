// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <boost/test/unit_test.hpp>

#include <iml/testing/random/AllocationHandle.hpp>
#include <iml/vmem/tmmgr.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <vector>

#define REQUIRE_ALLOC_SUCCESS(h,s,o,g)                                  \
  do                                                                    \
  {                                                                     \
    std::pair<iml_client::vmem::Offset_t, iml_client::vmem::MemSize_t> const        \
      OffsetSize (tmmgr.alloc (h, s));                                  \
                                                                        \
    BOOST_REQUIRE_EQUAL (OffsetSize.first, o);                          \
    BOOST_REQUIRE_EQUAL (OffsetSize.second, g);                         \
  }                                                                     \
  while (false)

BOOST_AUTO_TEST_CASE (tmmgr_works)
{
  iml_client::vmem::tmmgr tmmgr (45, 1);

  BOOST_REQUIRE_EQUAL (tmmgr.memsize(), 45);
  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 45);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 0);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 0);

  auto const handles
    ( fhg::util::testing::unique_randoms<std::vector<iml::AllocationHandle>>
        (14)
    );

  for (decltype (handles)::size_type i (0); i < 10; ++i)
  {
    REQUIRE_ALLOC_SUCCESS (handles[i], 1, i, 1);
  }

  tmmgr.free (handles[2]);
  tmmgr.free (handles[6]);
  tmmgr.free (handles[3]);
  tmmgr.free (handles[7]);
  tmmgr.free (handles[8]);
  tmmgr.free (handles[1]);
  tmmgr.free (handles[5]);
  tmmgr.free (handles[4]);

  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 43);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 10);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 2);

  REQUIRE_ALLOC_SUCCESS (handles[1], 1, 1, 1);

  REQUIRE_ALLOC_SUCCESS (handles[11], 4, 2, 4);
  REQUIRE_ALLOC_SUCCESS (handles[12], 4, 10, 4);

  fhg::util::testing::require_exception
    ( [&] { tmmgr.alloc (handles[11], 4); }
    , iml_client::vmem::error::alloc::duplicate_handle (handles[11])
    );
  fhg::util::testing::require_exception
    ( [&] { tmmgr.alloc (handles[12], 4); }
    , iml_client::vmem::error::alloc::duplicate_handle (handles[12])
    );
  fhg::util::testing::require_exception
    ( [&] { tmmgr.alloc (handles[13], 35); }
    , iml_client::vmem::error::alloc::insufficient_memory
        (handles[13], 35, 35, 34)
    );
  fhg::util::testing::require_exception
    ( [&] { tmmgr.alloc (handles[13], 34); }
    , iml_client::vmem::error::alloc::insufficient_contiguous_memory
        (handles[13], 34, 34, 34)
    );

  fhg::util::testing::require_exception
    ( [&] { tmmgr.free (handles[13]); }
    , iml_client::vmem::error::unknown_handle (handles[13])
    );

  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 34);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 14);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 5);
}

BOOST_AUTO_TEST_CASE (tmmgr_aligned)
{
  iml_client::vmem::tmmgr tmmgr (45, (1 << 4));

  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 32);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 0);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 0);

  tmmgr.resize (67);

  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 64);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 0);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 0);

  tmmgr.resize (64);

  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 64);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 0);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 0);

  auto const handles
    ( fhg::util::testing::unique_randoms<std::vector<iml::AllocationHandle>>
        (10)
    );

  REQUIRE_ALLOC_SUCCESS (handles[0], 1, 0, 16);
  REQUIRE_ALLOC_SUCCESS (handles[1], 1, 16, 16);
  REQUIRE_ALLOC_SUCCESS (handles[2], 1, 32, 16);
  REQUIRE_ALLOC_SUCCESS (handles[3], 1, 48, 16);

  fhg::util::testing::require_exception
    ( [&] { tmmgr.alloc (handles[4], 1); }
    , iml_client::vmem::error::alloc::insufficient_memory (handles[4], 1, 16, 0)
    );
  fhg::util::testing::require_exception
    ( [&] { tmmgr.alloc (handles[5], 1); }
    , iml_client::vmem::error::alloc::insufficient_memory (handles[5], 1, 16, 0)
    );
  fhg::util::testing::require_exception
    ( [&] { tmmgr.alloc (handles[6], 1); }
    , iml_client::vmem::error::alloc::insufficient_memory (handles[6], 1, 16, 0)
    );
  fhg::util::testing::require_exception
    ( [&] { tmmgr.alloc (handles[7], 1); }
    , iml_client::vmem::error::alloc::insufficient_memory (handles[7], 1, 16, 0)
    );
  fhg::util::testing::require_exception
    ( [&] { tmmgr.alloc (handles[8], 1); }
    , iml_client::vmem::error::alloc::insufficient_memory (handles[8], 1, 16, 0)
    );
  fhg::util::testing::require_exception
    ( [&] { tmmgr.alloc (handles[9], 1); }
    , iml_client::vmem::error::alloc::insufficient_memory (handles[9], 1, 16, 0)
    );

  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 0);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 64);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 4);

  tmmgr.resize (1000);

  fhg::util::testing::require_exception
    ( [&] { tmmgr.alloc (handles[0], 1); }
    , iml_client::vmem::error::alloc::duplicate_handle (handles[0])
    );
  fhg::util::testing::require_exception
    ( [&] { tmmgr.alloc (handles[1], 1); }
    , iml_client::vmem::error::alloc::duplicate_handle (handles[1])
    );
  fhg::util::testing::require_exception
    ( [&] { tmmgr.alloc (handles[2], 1); }
    , iml_client::vmem::error::alloc::duplicate_handle (handles[2])
    );
  fhg::util::testing::require_exception
    ( [&] { tmmgr.alloc (handles[3], 1); }
    , iml_client::vmem::error::alloc::duplicate_handle (handles[3])
    );

  REQUIRE_ALLOC_SUCCESS (handles[4], 1, 64, 16);
  REQUIRE_ALLOC_SUCCESS (handles[5], 1, 80, 16);
  REQUIRE_ALLOC_SUCCESS (handles[6], 1, 96, 16);
  REQUIRE_ALLOC_SUCCESS (handles[7], 1, 112, 16);
  REQUIRE_ALLOC_SUCCESS (handles[8], 1, 128, 16);
  REQUIRE_ALLOC_SUCCESS (handles[9], 1, 144, 16);

  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 832);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 160);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 10);

  fhg::util::testing::require_exception
    ( [&] { tmmgr.resize (150); }
    , iml_client::vmem::error::resize::below_mem_used (150, 144, 992, 832)
    );

  tmmgr.free (handles[0]);

  fhg::util::testing::require_exception
    ( [&] { tmmgr.resize (150); }
    , iml_client::vmem::error::resize::below_high_water (150, 144, 160)
    );
}
