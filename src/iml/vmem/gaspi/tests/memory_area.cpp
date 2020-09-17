#include <iml/vmem/gaspi/pc/memory/shm_area.hpp>

#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>
#include <iml/vmem/gaspi/pc/segment/segment.hpp>
#include <iml/vmem/gaspi/pc/type/types.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>

BOOST_AUTO_TEST_CASE ( memory_area_alloc_free )
{
  gpi::pc::memory::handle_generator_t handle_generator (42);

  auto const segm_size (2048u);
  auto const alloc_size (64u);

  gpi::pc::segment::segment_t segm ( "memory_area_alloc_free_test"
                                   , segm_size
                                   );
  segm.create ();
  gpi::pc::memory::shm_area_t area ( "memory_area_alloc_free_test"
                                   , segm_size
                                   , handle_generator
                                   );

  gpi::pc::type::handle_t hdl (area.alloc(alloc_size, "scratch", gpi::pc::is_global::no, 2));
  BOOST_CHECK_NE (hdl, gpi::pc::type::handle_t());

  std::cout << "    handle = " << hdl << std::endl;
  gpi::pc::type::handle::descriptor_t desc (area.descriptor (hdl));
  BOOST_CHECK_EQUAL (desc.id, hdl);
  BOOST_CHECK_EQUAL (desc.offset, segm_size - alloc_size);
  BOOST_CHECK_EQUAL (desc.size, alloc_size);
  BOOST_CHECK_EQUAL (desc.name, "scratch");
  BOOST_CHECK (desc.flags == gpi::pc::is_global::no);

  area.free (hdl);
}
