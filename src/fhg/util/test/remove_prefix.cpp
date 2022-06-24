// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <boost/test/unit_test.hpp>

#include <fhg/util/remove_prefix.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>

namespace
{
  const std::string word ("ababab");
}

BOOST_AUTO_TEST_CASE (different_length_prefixes)
{
  {
    const std::string prefix ("");

    BOOST_REQUIRE_EQUAL (fhg::util::remove_prefix (prefix, word), "ababab");
  }

  {
    const std::string prefix ("a");

    BOOST_REQUIRE_EQUAL (fhg::util::remove_prefix (prefix, word), "babab");
  }

  {
    const std::string prefix ("abab");

    BOOST_REQUIRE_EQUAL (fhg::util::remove_prefix (prefix, word), "ab");
  }

  {
    const std::string prefix ("ababab");

    BOOST_REQUIRE_EQUAL (fhg::util::remove_prefix (prefix, word), "");
  }
}

BOOST_AUTO_TEST_CASE (longer_prefix_than_input)
{
  const std::string prefix ("abababab");

  try
  {
    fhg::util::remove_prefix (prefix, word);

    BOOST_FAIL ("should throw");
  }
  catch (fhg::util::remove_prefix_failed const& f)
  {
    BOOST_REQUIRE_EQUAL (f.word(), "");
    BOOST_REQUIRE_EQUAL (f.prefix(), "ab");
  }
}

BOOST_AUTO_TEST_CASE (non_matched_prefix)
{
  const std::string prefix ("A");

  try
  {
    fhg::util::remove_prefix (prefix, word);

    BOOST_FAIL ("should throw");
  }
  catch (fhg::util::remove_prefix_failed const& f)
  {
    BOOST_REQUIRE_EQUAL (f.word(), "ababab");
    BOOST_REQUIRE_EQUAL (f.prefix(), "A");
  }
}
