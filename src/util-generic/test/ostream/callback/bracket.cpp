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

#include <util-generic/ostream/callback/bracket.hpp>
#include <util-generic/ostream/callback/print.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      namespace callback
      {
        BOOST_AUTO_TEST_CASE (bracket_surrounds_with_given_separators)
        {
          BOOST_REQUIRE_EQUAL
            (print<int> (bracket<int> ('[', ']'), 0).string(), "[0]");
          BOOST_REQUIRE_EQUAL
            (print<int> (bracket<int> ("~|", "|~"), 1).string(), "~|1|~");
          BOOST_REQUIRE_EQUAL
            (print<int> (bracket<int> (0, 1), 8).string(), "081");
        }
      }
    }
  }
}
