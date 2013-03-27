#define BOOST_TEST_MODULE job_id

#include <boost/test/unit_test.hpp>
#include <sstream>
#include <sdpa/JobId.hpp>

BOOST_AUTO_TEST_CASE (default_constructor)
{
  const sdpa::JobId jid1;
  const sdpa::JobId jid2;
  const sdpa::JobId jid3;

  BOOST_REQUIRE (jid1 != jid2);
  BOOST_REQUIRE (jid1 != jid3);
  BOOST_REQUIRE (jid2 != jid3);
}

BOOST_AUTO_TEST_CASE (conversion_from_string)
{
  const std::string EXPECTED ("010203040506070809101112131415");
  const sdpa::JobId jid1 (EXPECTED);
  BOOST_REQUIRE_EQUAL (EXPECTED, jid1.str());

  const sdpa::JobId jid2 ("foo");
  BOOST_REQUIRE_EQUAL (std::string("foo"), jid2.str());

  const sdpa::JobId jid3 (std::string("bar"));
  BOOST_REQUIRE_EQUAL (std::string("bar"), jid3.str());
}

BOOST_AUTO_TEST_CASE (conversion_to_string)
{
  const std::string EXPECTED ("010203040506070809101112131415");
  const sdpa::JobId jid (EXPECTED);
  const std::string actual (jid);
  BOOST_REQUIRE_EQUAL (EXPECTED, actual);

  const std::string jidString (jid);
  BOOST_REQUIRE_EQUAL (jidString, jid.str());
}

BOOST_AUTO_TEST_CASE (streaming_operator)
{
  const sdpa::JobId jid;
  std::ostringstream sstr;
  sstr << jid;

  BOOST_REQUIRE_EQUAL (jid.str(), sstr.str());
}
