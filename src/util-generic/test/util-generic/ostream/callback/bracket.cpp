// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
