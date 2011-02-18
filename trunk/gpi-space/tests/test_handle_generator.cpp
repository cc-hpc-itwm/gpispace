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
  BOOST_CHECK_EQUAL (globl.type, gpi::pc::type::segment::SEG_GLOBAL);
  BOOST_CHECK_EQUAL (globl.global.ident, 42U);
  BOOST_CHECK_EQUAL (globl.global.cntr, 1U);
  std::cerr << "global=" << globl << std::endl;

  gpi::pc::type::handle_t local (handle_generator_t::get().next (2));
  BOOST_CHECK_EQUAL (local.type, gpi::pc::type::segment::SEG_LOCAL);
  BOOST_CHECK_EQUAL (local.local.cntr, 1U);
  std::cerr << " local=" << local << std::endl;

  gpi::pc::type::handle_t shared (handle_generator_t::get().next (3));
  BOOST_CHECK_EQUAL (shared.type, gpi::pc::type::segment::SEG_SHARED);
  BOOST_CHECK_EQUAL (shared.local.cntr, 1U);
  std::cerr << "shared=" << shared << std::endl;

  gpi::pc::type::handle_id_t id = shared;
  BOOST_CHECK_EQUAL (shared, id);
}

BOOST_AUTO_TEST_CASE ( test_generate_interleaved )
{
  using namespace gpi::pc::memory;

  for (size_t i (0); i < 100; ++i)
  {
    gpi::pc::type::handle_t s (handle_generator_t::get().next (3));
    BOOST_CHECK_EQUAL (s.type, gpi::pc::type::segment::SEG_SHARED);
    BOOST_CHECK_EQUAL (s.local.cntr, i+1);

    gpi::pc::type::handle_t l (handle_generator_t::get().next (2));
    BOOST_CHECK_EQUAL (l.type, gpi::pc::type::segment::SEG_LOCAL);
    BOOST_CHECK_EQUAL (l.local.cntr, i+1);

    gpi::pc::type::handle_t g (handle_generator_t::get().next (1));
    BOOST_CHECK_EQUAL (g.type, gpi::pc::type::segment::SEG_GLOBAL);
    BOOST_CHECK_EQUAL (g.global.cntr, i+1);
  }
}

BOOST_AUTO_TEST_SUITE_END()
