// Copyright (C) 2013-2016,2020-2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/util/position.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>

namespace
{
  const char input[] =
    " 123456789 123456789 123456789 123456789 123456789\n"
    " 123456789 123456789 123456789 123456789 123456789\n"
    ;
}

BOOST_AUTO_TEST_CASE (path)
{
  std::filesystem::path const p {"/some/path"};
  const gspc::xml::parse::util::position_type position (nullptr, nullptr, p);

  BOOST_REQUIRE_EQUAL (position.path(), p);
}

BOOST_AUTO_TEST_CASE (null)
{
  const gspc::xml::parse::util::position_type position (nullptr, nullptr, "");

  BOOST_REQUIRE_EQUAL (position.line(), 1U);
  BOOST_REQUIRE_EQUAL (position.column(), 0U);
}

BOOST_AUTO_TEST_CASE (nothing)
{
  const gspc::xml::parse::util::position_type position (input, input, "");

  BOOST_REQUIRE_EQUAL (position.line(), 1U);
  BOOST_REQUIRE_EQUAL (position.column(), 0U);
}

BOOST_AUTO_TEST_CASE (inside_line)
{
  const gspc::xml::parse::util::position_type position (input, input + 9, "");

  BOOST_REQUIRE_EQUAL (position.line(), 1U);
  BOOST_REQUIRE_EQUAL (position.column(), 9U);
}

BOOST_AUTO_TEST_CASE (end_line)
{
  const gspc::xml::parse::util::position_type position (input, input + 50, "");

  BOOST_REQUIRE_EQUAL (position.line(), 1U);
  BOOST_REQUIRE_EQUAL (position.column(), 50U);
}

BOOST_AUTO_TEST_CASE (wrap_line)
{
  const gspc::xml::parse::util::position_type position (input, input + 51, "");

  BOOST_REQUIRE_EQUAL (position.line(), 2U);
  BOOST_REQUIRE_EQUAL (position.column(), 0U);
}

BOOST_AUTO_TEST_CASE (operator_out)
{
  std::filesystem::path const p {"/some/path"};
  const gspc::xml::parse::util::position_type position (input, input + 53, p);

  std::ostringstream oss;

  oss << position;

  BOOST_REQUIRE_EQUAL (oss.str(), std::string ("[/some/path:2:2]"));
}
