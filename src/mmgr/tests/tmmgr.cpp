// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE mmgr_tmmgr
#include <boost/test/unit_test.hpp>

#include <mmgr/tmmgr.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

#define REQUIRE_ALLOC_SUCCESS(h,s,o,g)                                  \
  {                                                                     \
    tmmgr.alloc (h, s);                                                 \
                                                                        \
    std::pair<gspc::vmem::Offset_t, gspc::vmem::MemSize_t> const        \
      OffsetSize (tmmgr.offset_size (h));                               \
                                                                        \
    BOOST_REQUIRE_EQUAL (OffsetSize.first, o);                          \
    BOOST_REQUIRE_EQUAL (OffsetSize.second, g);                         \
  }

BOOST_AUTO_TEST_CASE (tmmgr)
{
  gspc::vmem::tmmgr tmmgr (45, 1);

  BOOST_REQUIRE_EQUAL (tmmgr.memsize(), 45);
  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 45);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 0);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 0);

  for (int i (0); i < 10; ++i)
  {
    REQUIRE_ALLOC_SUCCESS (i, 1, i, 1);
  }

  tmmgr.free (2);
  tmmgr.free (6);
  tmmgr.free (3);
  tmmgr.free (7);
  tmmgr.free (8);
  tmmgr.free (1);
  tmmgr.free (5);
  tmmgr.free (4);

  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 43);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 10);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 2);

  REQUIRE_ALLOC_SUCCESS (1, 1, 1, 1);

  REQUIRE_ALLOC_SUCCESS (11, 4, 2, 4);
  REQUIRE_ALLOC_SUCCESS (12, 4, 10, 4);

  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.alloc (11, 4); }
    , gspc::vmem::error::alloc::duplicate_handle (11)
    );
  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.alloc (12, 4); }
    , gspc::vmem::error::alloc::duplicate_handle (12)
    );
  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.alloc (13, 35); }
    , gspc::vmem::error::alloc::insufficient_memory (13, 35, 35, 34)
    );
  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.alloc (13, 34); }
    , gspc::vmem::error::alloc::insufficient_contiguous_memory (13, 34, 34, 34)
    );

  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.free (13); }
    , gspc::vmem::error::unknown_handle (13)
    );

  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 34);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 14);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 5);
}

BOOST_AUTO_TEST_CASE (tmmgr_aligned)
{
  gspc::vmem::tmmgr tmmgr (45, (1 << 4));

  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 32);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 0);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 0);

  BOOST_REQUIRE_EQUAL (tmmgr.resize (67), gspc::vmem::tmmgr::RESIZE_SUCCESS);

  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 64);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 0);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 0);

  BOOST_REQUIRE_EQUAL (tmmgr.resize (64), gspc::vmem::tmmgr::RESIZE_SUCCESS);

  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 64);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 0);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 0);

  REQUIRE_ALLOC_SUCCESS (0, 1, 0, 16);
  REQUIRE_ALLOC_SUCCESS (1, 1, 16, 16);
  REQUIRE_ALLOC_SUCCESS (2, 1, 32, 16);
  REQUIRE_ALLOC_SUCCESS (3, 1, 48, 16);

  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.alloc (4, 1); }
    , gspc::vmem::error::alloc::insufficient_memory (4, 1, 16, 0)
    );
  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.alloc (5, 1); }
    , gspc::vmem::error::alloc::insufficient_memory (5, 1, 16, 0)
    );
  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.alloc (6, 1); }
    , gspc::vmem::error::alloc::insufficient_memory (6, 1, 16, 0)
    );
  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.alloc (7, 1); }
    , gspc::vmem::error::alloc::insufficient_memory (7, 1, 16, 0)
    );
  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.alloc (8, 1); }
    , gspc::vmem::error::alloc::insufficient_memory (8, 1, 16, 0)
    );
  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.alloc (9, 1); }
    , gspc::vmem::error::alloc::insufficient_memory (9, 1, 16, 0)
    );

  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 0);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 64);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 4);

  BOOST_REQUIRE_EQUAL (tmmgr.resize (1000), gspc::vmem::tmmgr::RESIZE_SUCCESS);

  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.alloc (0, 1); }
    , gspc::vmem::error::alloc::duplicate_handle (0)
    );
  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.alloc (1, 1); }
    , gspc::vmem::error::alloc::duplicate_handle (1)
    );
  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.alloc (2, 1); }
    , gspc::vmem::error::alloc::duplicate_handle (2)
    );
  fhg::util::testing::require_exception
    ( [&tmmgr]() { tmmgr.alloc (3, 1); }
    , gspc::vmem::error::alloc::duplicate_handle (3)
    );

  REQUIRE_ALLOC_SUCCESS (4, 1, 64, 16);
  REQUIRE_ALLOC_SUCCESS (5, 1, 80, 16);
  REQUIRE_ALLOC_SUCCESS (6, 1, 96, 16);
  REQUIRE_ALLOC_SUCCESS (7, 1, 112, 16);
  REQUIRE_ALLOC_SUCCESS (8, 1, 128, 16);
  REQUIRE_ALLOC_SUCCESS (9, 1, 144, 16);

  BOOST_REQUIRE_EQUAL (tmmgr.memfree(), 832);
  BOOST_REQUIRE_EQUAL (tmmgr.highwater(), 160);
  BOOST_REQUIRE_EQUAL (tmmgr.numhandle(), 10);

  BOOST_REQUIRE_EQUAL (tmmgr.resize (150), gspc::vmem::tmmgr::RESIZE_BELOW_MEMUSED);

  tmmgr.free (0);

  BOOST_REQUIRE_EQUAL (tmmgr.resize (150), gspc::vmem::tmmgr::RESIZE_BELOW_HIGHWATER);
}
