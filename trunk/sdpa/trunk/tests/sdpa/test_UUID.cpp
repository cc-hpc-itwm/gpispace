#include <iostream>

#include <string>
#include <map>

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
  CPPUNIT_ASSERT_EQUAL(36U, id.str().size());
}

void UUIDTest::testUniqueness()
{
  typedef std::map<std::string, bool> uuid_list_t;
  const std::size_t SAMPLES(10000);

  sdpa::uuid id;
  sdpa::uuidgen gen;
  uuid_list_t uuids;

  for (std::size_t i(0); i < SAMPLES; ++i) {
    gen(id);
    uuid_list_t::const_iterator it(uuids.find(id.str()));
    CPPUNIT_ASSERT_MESSAGE(std::string("generated uuid was not unique: ") + id.str(), it == uuids.end());
    uuids.insert(std::make_pair(id.str(), true));
  }
}

void UUIDTest::testLength() {
  sdpa::uuid id;
  sdpa::uuidgen gen;
  const std::size_t SAMPLES(10000);
  const std::size_t EXPECTED_LENGTH(36);
  for (std::size_t sample = 0; sample < SAMPLES; sample++)
  {
    gen(id);
    CPPUNIT_ASSERT_EQUAL(EXPECTED_LENGTH, id.str().size());
  }
}

