// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <gpi-space/pc/memory/shm_area.hpp>

#include <gpi-space/pc/memory/handle_generator.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/type/flags.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>

BOOST_AUTO_TEST_CASE ( memory_area_alloc_free )
{
  gpi::pc::memory::handle_generator_t handle_generator (42);

  gpi::pc::segment::segment_t segm ( "memory_area_alloc_free_test"
                                   , 2048
                                   );
  segm.create ();
  fhg::logging::stream_emitter logger;
  gpi::pc::memory::shm_area_t area ( logger
                                   , 0
                                   , "memory_area_alloc_free_test"
                                   , 2048
                                   , handle_generator
                                   );
  area.set_id (2);

  BOOST_CHECK_EQUAL (2048U, area.descriptor().local_size);
  BOOST_CHECK_EQUAL (2048U, area.descriptor().avail);

  gpi::pc::type::handle_t hdl (area.alloc(1, 64, "scratch", 0));
  BOOST_CHECK_NE (hdl, gpi::pc::type::handle_t());
  BOOST_CHECK_EQUAL (2048u, area.descriptor().local_size);
  BOOST_CHECK_EQUAL (2048u - 64, area.descriptor().avail);

  std::cout << "    handle = " << hdl << std::endl;
  gpi::pc::type::handle::descriptor_t desc (area.descriptor (hdl));
  BOOST_CHECK_EQUAL (desc.id.handle, hdl.handle);
  BOOST_CHECK_EQUAL (desc.segment, area.descriptor().id);
  BOOST_CHECK_EQUAL (desc.offset, 0u);
  BOOST_CHECK_EQUAL (desc.size, 64u);
  BOOST_CHECK_EQUAL (desc.name, "scratch");
  BOOST_CHECK_EQUAL (desc.flags, 0u);

  area.free (hdl);
  BOOST_CHECK_EQUAL (2048u, area.descriptor().local_size);
  BOOST_CHECK_EQUAL (2048u, area.descriptor().avail);
}
