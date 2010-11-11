#define BOOST_TEST_MODULE PeerTest
#include <boost/test/unit_test.hpp>

#include <fhglog/minimal.hpp>

#include <gpi-space/config/node_config.hpp>
#include <gpi-space/config/node_config_io.hpp>

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
  using namespace gpi_space::config;
  node_config_t nc;
  std::ostringstream s;
  s << nc;
  LOG(DEBUG, "cfg = " << s.str());
  BOOST_CHECK_EQUAL (s.str(), "");
}

BOOST_AUTO_TEST_SUITE_END()
