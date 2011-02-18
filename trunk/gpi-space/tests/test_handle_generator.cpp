#define BOOST_TEST_MODULE GpiSpaceHandleGeneratorTest
#include <boost/test/unit_test.hpp>

#include <fhglog/minimal.hpp>

#include <gpi-space/pc/memory/handle_generator.hpp>
#include <gpi-space/pc/type/segment_descriptor.hpp>

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

BOOST_AUTO_TEST_CASE ( test_generate )
{
  using namespace gpi::pc::memory;

  gpi::pc::type::handle_t inval (handle_generator_t::get().next (0));
  BOOST_CHECK_EQUAL (inval, 0);
  std::cerr << " inval=" << inval << std::endl;

  gpi::pc::type::handle_t globl (handle_generator_t::get().next (1));
  BOOST_CHECK_EQUAL (globl.global.type, gpi::pc::type::segment::SEG_GLOBAL);
  BOOST_CHECK_EQUAL (globl.global.ident, 42U);
  BOOST_CHECK_EQUAL (globl.global.cntr, 1U);
  std::cerr << "global=" << globl << std::endl;

  gpi::pc::type::handle_t local (handle_generator_t::get().next (2));
  BOOST_CHECK_EQUAL (local.local.type, gpi::pc::type::segment::SEG_LOCAL);
  BOOST_CHECK_EQUAL (local.local.cntr, 2U);
  std::cerr << " local=" << local << std::endl;

  gpi::pc::type::handle_t shared (handle_generator_t::get().next (3));
  BOOST_CHECK_EQUAL (shared.local.type, gpi::pc::type::segment::SEG_SHARED);
  BOOST_CHECK_EQUAL (shared.local.cntr, 3U);
  std::cerr << "shared=" << shared << std::endl;

  gpi::pc::type::handle_id_t id = shared;
  BOOST_CHECK_EQUAL (shared, id);
}

BOOST_AUTO_TEST_SUITE_END()
