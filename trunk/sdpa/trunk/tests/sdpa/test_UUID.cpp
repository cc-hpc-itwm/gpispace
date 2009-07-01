#include <iostream>

#include "test_UUID.hpp"
#include <sdpa/util.hpp>
#include <sdpa/uuid.hpp>
#include <sdpa/uuidgen.hpp>

using namespace sdpa;
using namespace sdpa::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( sdpa::tests::UUIDTest );

void UUIDTest::setUp() {
}

void UUIDTest::tearDown() {
}

void UUIDTest::testGenerate() {
  sdpa::uuid id;
  sdpa::uuidgen gen;
  gen(id);
  std::clog << "generated " << id.str() << std::endl;
}
