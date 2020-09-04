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
