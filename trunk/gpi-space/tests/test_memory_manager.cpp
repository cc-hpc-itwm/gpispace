#include <stdio.h> // snprintf

#define BOOST_TEST_MODULE GpiSpaceMemoryManagerTest
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include <fhglog/minimal.hpp>

#include <gpi-space/pc/memory/handle_generator.hpp>
#include <gpi-space/pc/memory/memory_area.hpp>

typedef boost::shared_ptr<gpi::pc::segment::segment_t> segment_ptr;

struct SetupLogging
{
  SetupLogging()
  {
    FHGLOG_SETUP();
    BOOST_TEST_MESSAGE ("setup logging");
  }

  ~SetupLogging()
  {}
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
  segment_ptr seg
    (new gpi::pc::segment::segment_t("memory_area_alloc_free_segment"
                                    , 1024
                                    , gpi::pc::type::segment::SEG_GLOBAL
                                    )
    );
  seg->create ();
  seg->unlink ();

  gpi::pc::memory::area_t area(seg);
  BOOST_CHECK_EQUAL (area.total_mem_size(), seg->size());
  BOOST_CHECK_EQUAL (area.free_mem_size(), seg->size());
  BOOST_CHECK_EQUAL (area.used_mem_size(), 0U);

  gpi::pc::type::handle_id_t hdl
      (area.alloc(1, 64, "scratch"));
  BOOST_CHECK ( hdl != 0 );
  BOOST_CHECK_EQUAL (area.total_mem_size(), seg->size());
  BOOST_CHECK_EQUAL (area.free_mem_size(), seg->size() - 64);
  BOOST_CHECK_EQUAL (area.used_mem_size(), 64U);

  std::cout << "    handle = " << gpi::pc::type::handle_t(hdl) << std::endl;
  gpi::pc::type::handle::descriptor_t desc;
  BOOST_CHECK_EQUAL (area.get_descriptor (hdl, desc), true);
  BOOST_CHECK_EQUAL (desc.id, hdl);
  BOOST_CHECK_EQUAL (desc.segment, seg->id());
  BOOST_CHECK_EQUAL (desc.offset, (seg->size() - 64));
  BOOST_CHECK_EQUAL (desc.size, 64u);
  BOOST_CHECK_EQUAL (desc.name, "scratch");
  BOOST_CHECK_EQUAL (desc.flags, 0);

  gpi::pc::type::handle::list_t list;
  area.list_allocations (list);
  BOOST_CHECK_EQUAL (list.size(), 1u);

  std::cout << "descriptor = " << desc << std::endl;

  area.free (hdl);
  BOOST_CHECK_EQUAL (area.total_mem_size(), seg->size());
  BOOST_CHECK_EQUAL (area.free_mem_size(), seg->size());
  BOOST_CHECK_EQUAL (area.used_mem_size(), 0U);
}

BOOST_AUTO_TEST_SUITE_END()
