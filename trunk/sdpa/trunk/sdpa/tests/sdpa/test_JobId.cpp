#include <iostream>
#include <sstream>

#include "test_JobId.hpp"
#include <sdpa/util/util.hpp>
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
  CPPUNIT_ASSERT_EQUAL(EXPECTED, jid1.str());

  JobId jid2 = "foo";
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), jid2.str());

  JobId jid3 = std::string("bar");
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), jid3.str());
}

void JobIdTest::testAutoConversionToString() {
  const std::string EXPECTED("010203040506070809101112131415");
  JobId jid(EXPECTED);
  const std::string actual(jid);
  CPPUNIT_ASSERT_EQUAL(EXPECTED, actual);

  const std::string jidString = jid;
  CPPUNIT_ASSERT_EQUAL(jidString, jid.str());
}

void JobIdTest::testStream() {
  JobId jid;
  std::ostringstream sstr;
  sstr << jid;

  CPPUNIT_ASSERT_EQUAL(jid.str(), sstr.str());
}
