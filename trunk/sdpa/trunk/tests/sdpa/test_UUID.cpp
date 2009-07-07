#include <iostream>

#include <string>
#include <list>

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
  CPPUNIT_ASSERT_EQUAL(std::size_t(32), id.str().size());
}

void UUIDTest::testUniqueness()
{
  typedef std::list<std::string> uuid_list_t;
  const std::size_t SAMPLES(10000);

  sdpa::uuid id;
  sdpa::uuidgen gen;
  uuid_list_t uuids;
  for (std::size_t i(0); i < SAMPLES; ++i) {
    gen(id);
    for (uuid_list_t::const_iterator it(uuids.begin()); it != uuids.end(); ++it) {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("uuid was not unique!", *it, id.str());
    }
    uuids.push_back(id.str());
  }
}

void UUIDTest::testLength() {
  sdpa::uuid id;
  sdpa::uuidgen gen;
  const std::size_t SAMPLES(10000);
  const std::size_t EXPECTED_LENGTH(32);
  for (std::size_t sample = 0; sample < SAMPLES; sample++)
  {
    gen(id);
    CPPUNIT_ASSERT_EQUAL(EXPECTED_LENGTH, id.str().size());
  }
}

