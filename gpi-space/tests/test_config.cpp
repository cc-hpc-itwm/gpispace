#define BOOST_TEST_MODULE GpiSpaceConfigTest
#include <boost/test/unit_test.hpp>

#include <fhglog/minimal.hpp>

#include <gpi-space/config/config.hpp>
#include <gpi-space/config/config_io.hpp>

struct SetupLogging
{
  SetupLogging ()
  {
    FHGLOG_SETUP();
  }
};

BOOST_FIXTURE_TEST_SUITE( setup_logging, SetupLogging )
BOOST_AUTO_TEST_SUITE_END()

struct F
{
  F()
  { }
};

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE ( config_io )
{
  gpi_space::config cfg;
  std::ostringstream s;
  s << cfg;
  LOG(DEBUG, "cfg = " << s.str());
}

BOOST_AUTO_TEST_SUITE_END()
