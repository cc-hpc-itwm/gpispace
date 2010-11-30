#define BOOST_TEST_MODULE TestConfig
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>

#include <sdpa/util/util.hpp>
#include <sdpa/util/Config.hpp>
#include <sdpa/logging.hpp>

using namespace sdpa;

struct MyFixture
{
	MyFixture() :SDPA_INIT_LOGGER("sdpa.tests.testConfig"){}
	~MyFixture(){}
	 SDPA_DECLARE_LOGGER();
};

BOOST_FIXTURE_TEST_SUITE( test_Config, MyFixture )

BOOST_AUTO_TEST_CASE( testPopulate)
{
	sdpa::util::Config::ptr_t cfg = sdpa::util::Config::create();
	sdpa::util::CommonConfiguration()(*cfg);
	cfg->parse_env();
	cfg->parse_file("...");
	cfg->parse("./program blah blah blah");

	BOOST_CHECK(cfg->is_set("sdpa.logging.level"));
}

BOOST_AUTO_TEST_SUITE_END()
