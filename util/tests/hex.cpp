#define BOOST_TEST_MODULE HEXTest
#include <boost/test/unit_test.hpp>

#include <fhg/util/hex.hpp>

#include <sstream>
#include <iostream>

BOOST_AUTO_TEST_CASE (test_to_hex)
{
  using namespace fhg::util;

  std::string bytes;
  bytes.push_back (0xde);
  bytes.push_back (0xad);
  bytes.push_back (0xbe);
  bytes.push_back (0xef);

  std::string expected ("deadbeef");

  BOOST_REQUIRE_EQUAL (to_hex (bytes), expected);
}

BOOST_AUTO_TEST_CASE (test_from_hex)
{
  using namespace fhg::util;

  std::string bytes;
  bytes.push_back (0xde);
  bytes.push_back (0xad);
  bytes.push_back (0xbe);
  bytes.push_back (0xef);

  BOOST_REQUIRE_EQUAL (from_hex ("deadbeef"), bytes);
}

BOOST_AUTO_TEST_CASE (test_fixpoint_hex)
{
  using namespace fhg::util;

  std::string bytes;
  bytes.push_back (0xde);
  bytes.push_back (0xad);
  bytes.push_back (0xbe);
  bytes.push_back (0xef);

  BOOST_REQUIRE_EQUAL (from_hex (to_hex (bytes)), bytes);
}
