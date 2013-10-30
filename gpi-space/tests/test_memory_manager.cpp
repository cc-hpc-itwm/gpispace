#include <stdio.h> // snprintf

#define BOOST_TEST_MODULE GpiSpaceMemoryManagerTest
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include <fhglog/minimal.hpp>

#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/memory/handle_generator.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/memory/shm_area.hpp>

struct SetupLogging
{
  SetupLogging()
  {
    FHGLOG_SETUP();
    BOOST_TEST_MESSAGE ("setup logging");
  }
};

BOOST_GLOBAL_FIXTURE( SetupLogging );

struct F
{
  F()
  {
    BOOST_TEST_MESSAGE ("fixture setup");
    gpi::pc::memory::handle_generator_t::create (42);
  }

  ~F ()
  {
    BOOST_TEST_MESSAGE ("fixture teardown");
    gpi::pc::memory::handle_generator_t::destroy ();
  }
};

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE ( memory_area_alloc_free )
{
  gpi::pc::segment::segment_t segm ( "memory_area_alloc_free_test"
                                   , 2048
                                   );
  segm.create ();
  gpi::pc::memory::shm_area_t area ( 0
                                   , "memory_area_alloc_free_test"
                                   , 2048
                                   , gpi::pc::F_NOCREATE
                                   );
  area.set_id (2);

  BOOST_CHECK_EQUAL (2048U, area.descriptor().local_size);
  BOOST_CHECK_EQUAL (2048U, area.descriptor().avail);

  gpi::pc::type::handle_t hdl (area.alloc(1, 64, "scratch", 0));
  BOOST_CHECK (hdl != 0);
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

  gpi::pc::type::handle::list_t list;
  area.list_allocations (1, list);
  BOOST_CHECK_EQUAL (list.size(), 1u);

  std::cout << "descriptor = " << desc << std::endl;

  area.free (hdl);
  BOOST_CHECK_EQUAL (2048u, area.descriptor().local_size);
  BOOST_CHECK_EQUAL (2048u, area.descriptor().avail);
}

BOOST_AUTO_TEST_SUITE_END()
