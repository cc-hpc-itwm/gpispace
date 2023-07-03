// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
