// Copyright (C) 2011-2012,2014-2015,2020-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/vmem/gaspi/pc/memory/shm_area.hpp>

#include <gspc/testing/iml/random/AllocationHandle.hpp>
#include <gspc/testing/iml/random/SegmentHandle.hpp>
#include <gspc/iml/vmem/gaspi/pc/memory/handle_generator.hpp>
#include <gspc/iml/vmem/gaspi/pc/segment/segment.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/random.hpp>

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

  auto const hdl (gspc::testing::random<gspc::iml::AllocationHandle>{}());
  auto const segment_id (gspc::testing::random<gspc::iml::SegmentHandle>{}());
  area.alloc (alloc_size, gpi::pc::is_global::no, segment_id, hdl);
  BOOST_CHECK_NE (hdl, gspc::iml::AllocationHandle());

  std::cout << "    handle = " << hdl << std::endl;
  gpi::pc::type::handle::descriptor_t desc (area.descriptor (hdl));
  BOOST_CHECK_EQUAL (desc.id, hdl);
  BOOST_CHECK_EQUAL (desc.offset, segm_size - alloc_size);
  BOOST_CHECK_EQUAL (desc.size, alloc_size);
  BOOST_CHECK (desc.flags == gpi::pc::is_global::no);

  area.free (hdl);
}
