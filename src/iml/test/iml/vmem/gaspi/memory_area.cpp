// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/vmem/gaspi/pc/memory/shm_area.hpp>

#include <iml/testing/random/AllocationHandle.hpp>
#include <iml/testing/random/SegmentHandle.hpp>
#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>
#include <iml/vmem/gaspi/pc/segment/segment.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <iostream>

BOOST_AUTO_TEST_CASE ( memory_area_alloc_free )
{
  auto const segm_size (2048u);
  auto const alloc_size (64u);

  gpi::pc::segment::segment_t segm ( "memory_area_alloc_free_test"
                                   , segm_size
                                   );
  segm.create ();
  gpi::pc::memory::shm_area_t area ( "memory_area_alloc_free_test"
                                   , segm_size
                                   );

  auto const hdl (fhg::util::testing::random<iml::AllocationHandle>{}());
  auto const segment_id (fhg::util::testing::random<iml::SegmentHandle>{}());
  area.alloc (alloc_size, gpi::pc::is_global::no, segment_id, hdl);
  BOOST_CHECK_NE (hdl, iml::AllocationHandle());

  std::cout << "    handle = " << hdl << std::endl;
  gpi::pc::type::handle::descriptor_t desc (area.descriptor (hdl));
  BOOST_CHECK_EQUAL (desc.id, hdl);
  BOOST_CHECK_EQUAL (desc.offset, segm_size - alloc_size);
  BOOST_CHECK_EQUAL (desc.size, alloc_size);
  BOOST_CHECK (desc.flags == gpi::pc::is_global::no);

  area.free (hdl);
}
