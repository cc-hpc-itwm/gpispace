#include <iostream>
#include <sstream>

#include "test_Config.hpp"
#include <sdpa/util/util.hpp>
#include <sdpa/util/Config.hpp>

using namespace sdpa;
using namespace sdpa::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( sdpa::tests::ConfigTest );

void ConfigTest::setUp() {
}

void ConfigTest::tearDown() {
}

void ConfigTest::testPopulate() {
  sdpa::util::Config::ptr_t cfg = sdpa::util::Config::create();
  sdpa::util::CommonConfiguration()(*cfg);
  cfg->parse_env();
  cfg->parse_file("...");
  cfg->parse("./program blah blah blah");

  CPPUNIT_ASSERT(cfg->is_set("sdpa.logging.level"));
}
