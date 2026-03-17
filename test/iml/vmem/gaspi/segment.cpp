// Copyright (C) 2011,2014-2015,2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/vmem/gaspi/pc/segment/segment.hpp>

#include <gspc/util/finally.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

#include <string>

BOOST_AUTO_TEST_CASE (sequence_of_create_close_unlink_does_not_throw)
{
  gpi::pc::segment::segment_t seg
    ( "seg-test-" + std::to_string (getpid())
    , 1024
    );

  seg.create();
  FHG_UTIL_FINALLY ([&] { seg.unlink(); });

  seg.close();
}
