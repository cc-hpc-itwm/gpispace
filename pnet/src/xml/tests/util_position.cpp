// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE parse_util_position

#include <xml/parse/util/position.hpp>

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>

namespace
{
  char* input =
    " 123456789 123456789 123456789 123456789 123456789\n"
    " 123456789 123456789 123456789 123456789 123456789\n"
    ;
}

BOOST_AUTO_TEST_CASE (path)
{
  const boost::filesystem::path p ("/some/path");
  const xml::parse::util::position_type position (0, 0, p);

  BOOST_REQUIRE_EQUAL (position.path(), p);
}

BOOST_AUTO_TEST_CASE (null)
{
  const xml::parse::util::position_type position (0, 0, "");

  BOOST_REQUIRE_EQUAL (position.line(), 1U);
  BOOST_REQUIRE_EQUAL (position.column(), 0U);
}

BOOST_AUTO_TEST_CASE (nothing)
{
  const xml::parse::util::position_type position (input, input, "");

  BOOST_REQUIRE_EQUAL (position.line(), 1U);
  BOOST_REQUIRE_EQUAL (position.column(), 0U);
}

BOOST_AUTO_TEST_CASE (inside_line)
{
  const xml::parse::util::position_type position (input, input + 9, "");

  BOOST_REQUIRE_EQUAL (position.line(), 1U);
  BOOST_REQUIRE_EQUAL (position.column(), 9U);
}

BOOST_AUTO_TEST_CASE (end_line)
{
  const xml::parse::util::position_type position (input, input + 50, "");

  BOOST_REQUIRE_EQUAL (position.line(), 1U);
  BOOST_REQUIRE_EQUAL (position.column(), 50U);
}

BOOST_AUTO_TEST_CASE (wrap_line)
{
  const xml::parse::util::position_type position (input, input + 51, "");

  BOOST_REQUIRE_EQUAL (position.line(), 2U);
  BOOST_REQUIRE_EQUAL (position.column(), 0U);
}

BOOST_AUTO_TEST_CASE (operator_out)
{
  const boost::filesystem::path p ("/some/path");
  const xml::parse::util::position_type position (input, input + 53, p);

  std::ostringstream oss;

  oss << position;

  BOOST_REQUIRE_EQUAL (oss.str(), std::string ("[/some/path:2:2]"));
}
