// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <util-generic/split.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <vector>
#include <string>

namespace
{
  std::list<std::string> split_at_dot (std::string path)
  {
    return fhg::util::split<std::string, std::string> (path, '.');
  }

  template<typename Convertable_to_string>
    void verify ( Convertable_to_string const& input
                , std::list<std::string> const& expected
                )
  {
    std::list<std::string> const result (split_at_dot (input));

    BOOST_REQUIRE_EQUAL_COLLECTIONS
      (result.begin(), result.end(), expected.begin(), expected.end());
  }
}

BOOST_AUTO_TEST_CASE (empty_string_results_in_empty_set)
{
  verify ("", {});
}

BOOST_AUTO_TEST_CASE (trailing_empty_element_is_ignored)
{
  verify ("foo.", {"foo"});
  verify ("fhg.log.", {"fhg", "log"});
}

BOOST_AUTO_TEST_CASE (non_trailing_empty_element_is_preserved)
{
  verify (".", {""});
  verify ("..", {"", ""});
  verify (".PHONY", {"", "PHONY"});
}

BOOST_AUTO_TEST_CASE (non_empty_elements_are_preserved)
{
  verify ("fhg.log.logger.1", {"fhg", "log", "logger", "1"});
}

BOOST_AUTO_TEST_CASE (no_separator_in_input)
{
  verify ("test", {"test"});
}
