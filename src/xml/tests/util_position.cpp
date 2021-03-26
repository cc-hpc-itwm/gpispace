// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <xml/parse/util/position.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

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
  const boost::filesystem::path p ("/some/path");
  const xml::parse::util::position_type position (nullptr, nullptr, p);

  BOOST_REQUIRE_EQUAL (position.path(), p);
}

BOOST_AUTO_TEST_CASE (null)
{
  const xml::parse::util::position_type position (nullptr, nullptr, "");

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
