#define BOOST_TEST_MODULE GpiSpaceConfigTest
#include <boost/test/unit_test.hpp>

#include <fhglog/LogMacros.hpp>

#include <gpi-space/config/config.hpp>

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

BOOST_AUTO_TEST_SUITE_END()
