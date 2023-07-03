// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/ostream/callback/open.hpp>
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
        BOOST_AUTO_TEST_CASE (open_prepends_given_separator)
        {
          BOOST_REQUIRE_EQUAL
            (print<int> (open<int> ('['), 0).string(), "[0");
          BOOST_REQUIRE_EQUAL
            (print<int> (open<int> ("~|"), 1).string(), "~|1");
          BOOST_REQUIRE_EQUAL
            (print<int> (open<int> (0), 8).string(), "08");
        }
      }
    }
  }
}
