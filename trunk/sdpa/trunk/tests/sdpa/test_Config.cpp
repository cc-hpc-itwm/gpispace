#include <iostream>
#include <sstream>

#include "test_Config.hpp"
#include <sdpa/util.hpp>
#include <sdpa/Config.hpp>

using namespace sdpa;
using namespace sdpa::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( sdpa::tests::ConfigTest );

void ConfigTest::setUp() {
}

void ConfigTest::tearDown() {
}

void ConfigTest::testPopulate() {
  sdpa::config::Config::ptr_t cfg = sdpa::config::Config::create();
  sdpa::config::CommonConfiguration()(*cfg);
  cfg->parse_env();
  cfg->parse_file("...");
  cfg->parse("./program blah blah blah");

  CPPUNIT_ASSERT(cfg->is_set("sdpa.logging.level"));
}
