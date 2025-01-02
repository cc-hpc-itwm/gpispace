// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/vmem/gaspi/pc/segment/segment.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>

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
