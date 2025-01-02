// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/ostream/callback/print.hpp>
#include <util-generic/ostream/callback/range.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

#include <list>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      namespace callback
      {
        using List = std::list<int>;
        using It = List::const_iterator;
        using Range = std::pair<It, It>;

        BOOST_AUTO_TEST_CASE (range_prints_nothing_for_empty_list)
        {
          List const list {};

          BOOST_REQUIRE_EQUAL
            ( print<Range> (range<It> ('-'), {list.begin(), list.end()}).string()
            , ""
            );
        }
        BOOST_AUTO_TEST_CASE
          (range_prints_only_element_for_single_element_list)
        {
          List const list {0};

          BOOST_REQUIRE_EQUAL
            ( print<Range> (range<It> ('-'), {list.begin(), list.end()}).string()
            , "0"
            );
        }
        BOOST_AUTO_TEST_CASE
          (range_prints_all_elements_separated_by_given_separator)
        {
          List list {0, 1};

          BOOST_REQUIRE_EQUAL
            ( print<Range> (range<It> ('-'), {list.begin(), list.end()}).string()
            , "0-1"
            );

          list.push_back (2);

          BOOST_REQUIRE_EQUAL
            ( print<Range> (range<It> ('-'), {list.begin(), list.end()}).string()
            , "0-1-2"
            );
        }
      }
    }
  }
}
