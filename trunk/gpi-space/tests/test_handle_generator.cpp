#define BOOST_TEST_MODULE GpiSpaceHandleGeneratorTest
#include <boost/test/unit_test.hpp>

#include <fhglog/minimal.hpp>

#include <gpi-space/pc/memory/handle_generator.hpp>

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

BOOST_AUTO_TEST_CASE ( generate_one )
{
  using namespace gpi::pc::memory;
  const size_t f (sizeof(gpi::pc::type::handle_id_t)*2);

  std::cout << std::setfill('0') << std::setw(f) << std::hex
            << handle_generator_t::get().next (1)
            << std::endl;
  std::cout << std::setfill('0') << std::setw(f) << std::hex
            << handle_generator_t::get().next (2)
            << std::endl;
  std::cout << std::setfill('0') << std::setw(f) << std::hex
            << handle_generator_t::get().next (3)
            << std::endl;
  std::cout << std::setfill('0') << std::setw(f) << std::hex
            << handle_generator_t::get().next (4)
            << std::endl;
  std::cout << std::setfill('0') << std::setw(f) << std::hex
            << handle_generator_t::get().next (5)
            << std::endl;
  std::cout << std::setfill('0') << std::setw(f) << std::hex
            << handle_generator_t::get().next (6)
            << std::endl;
  std::cout << std::setfill('0') << std::setw(f) << std::hex
            << handle_generator_t::get().next (7)
            << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
