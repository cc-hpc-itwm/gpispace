#include <stdio.h> // snprintf

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/memory/handle_generator.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/memory/shm_area.hpp>

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
