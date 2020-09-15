#include <iml/vmem/gaspi/pc/memory/shm_area.hpp>

#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>
#include <iml/vmem/gaspi/pc/segment/segment.hpp>
#include <iml/vmem/gaspi/pc/type/flags.hpp>

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
  gpi::pc::memory::shm_area_t area ( 0
                                   , "memory_area_alloc_free_test"
                                   , 2048
                                   , handle_generator
                                   );
  area.set_id (2);

  BOOST_CHECK_EQUAL (2048U, area.descriptor().local_size);
  BOOST_CHECK_EQUAL (2048U, area.descriptor().avail);

  gpi::pc::type::handle_t hdl (area.alloc(1, 64, "scratch", gpi::pc::is_global::no));
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
  BOOST_CHECK (desc.flags == gpi::pc::is_global::no);

  area.free (hdl);
  BOOST_CHECK_EQUAL (2048u, area.descriptor().local_size);
  BOOST_CHECK_EQUAL (2048u, area.descriptor().avail);
}
