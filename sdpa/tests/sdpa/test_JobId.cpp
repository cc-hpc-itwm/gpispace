#define BOOST_TEST_MODULE TestJobId
#include "sdpa/daemon/jobFSM/JobFSM.hpp"
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>

#include <sdpa/util/util.hpp>
#include <sdpa/JobId.hpp>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include <assert.h>

using namespace sdpa;

struct MyFixture
{
   MyFixture() :SDPA_INIT_LOGGER("sdpa.tests.testJobId"){}
   ~MyFixture(){}
   SDPA_DECLARE_LOGGER();
};

BOOST_FIXTURE_TEST_SUITE( test_JobId, MyFixture )

BOOST_AUTO_TEST_CASE(testDefaultConstructor)
{
  JobId jid1;
  JobId jid2;
  JobId jid3;

  BOOST_CHECK(jid1 != jid2);
  BOOST_CHECK(jid1 != jid3);
  BOOST_CHECK(jid2 != jid3);
}

BOOST_AUTO_TEST_CASE(testAutoConversionFromString)
{
  const std::string EXPECTED("010203040506070809101112131415");
  JobId jid1 = EXPECTED;
  BOOST_CHECK(EXPECTED == jid1.str());

  JobId jid2 = "foo";
  BOOST_CHECK(std::string("foo") == jid2.str());

  JobId jid3 = std::string("bar");
  BOOST_CHECK(std::string("bar") == jid3.str());
}

BOOST_AUTO_TEST_CASE(testAutoConversionToString)
{
  const std::string EXPECTED("010203040506070809101112131415");
  JobId jid(EXPECTED);
  const std::string actual(jid);
  BOOST_CHECK(EXPECTED == actual);

  const std::string jidString = jid;
  BOOST_CHECK(jidString == jid.str());
}

BOOST_AUTO_TEST_CASE(testStream)
{
  JobId jid;
  std::ostringstream sstr;
  sstr << jid;

  BOOST_CHECK(jid.str() == sstr.str());
}

BOOST_AUTO_TEST_SUITE_END()
