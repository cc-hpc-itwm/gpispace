#include <iostream>

#include "test_JobId.hpp"
#include <sdpa/util.hpp>
#include <sdpa/JobId.hpp>

using namespace sdpa;
using namespace sdpa::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( sdpa::tests::JobIdTest );

void JobIdTest::setUp() {
}

void JobIdTest::tearDown() {
}

void JobIdTest::testDefaultConstructor() {
  JobId jid1;
  JobId jid2;
  JobId jid3;

  CPPUNIT_ASSERT(jid1 != jid2);
  CPPUNIT_ASSERT(jid1 != jid3);
  CPPUNIT_ASSERT(jid2 != jid3);
}

void JobIdTest::testAutoConversionFromString() {
  const std::string EXPECTED("010203040506070809101112131415");
  JobId jid1 = EXPECTED;
  CPPUNIT_ASSERT_EQUAL(EXPECTED, std::string(jid1));

  JobId jid2 = "foo";
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), std::string(jid2));
}

void JobIdTest::testAutoConversionToString() {
  const std::string EXPECTED("010203040506070809101112131415");
  JobId jid(EXPECTED);
  const std::string actual = jid;
  CPPUNIT_ASSERT_EQUAL(EXPECTED, actual);
}
