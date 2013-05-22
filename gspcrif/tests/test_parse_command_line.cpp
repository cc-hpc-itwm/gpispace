#define BOOST_TEST_MODULE GspcRifCmdlineTests
#include <boost/test/unit_test.hpp>

struct F
{
  F ()
  {}
};

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE (test_simple_cmdline)
{
  BOOST_REQUIRE_MESSAGE (false, "not yet implemented");
}

BOOST_AUTO_TEST_SUITE_END()
