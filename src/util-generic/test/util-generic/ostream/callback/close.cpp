// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/ostream/callback/close.hpp>
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
        BOOST_AUTO_TEST_CASE (close_appends_given_separator)
        {
          BOOST_REQUIRE_EQUAL
            (print<int> (close<int> (']'), 0).string(), "0]");
          BOOST_REQUIRE_EQUAL
            (print<int> (close<int> ("|~"), 1).string(), "1|~");
          BOOST_REQUIRE_EQUAL
            (print<int> (close<int> (1), 8).string(), "81");
        }
      }
    }
  }
}
