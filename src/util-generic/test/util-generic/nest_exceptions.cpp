// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/nest_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE (does_nothing_if_not_throwing)
    {
      BOOST_REQUIRE
        (nest_exceptions<std::runtime_error> ([] { return true; }, "unused"));
    }

    BOOST_AUTO_TEST_CASE (wraps_exception_with_specified_type_and_args)
    {
      auto const inner (testing::random<std::string>{}());
      auto const outer (testing::random<std::string>{}());

      testing::require_exception
        ( [&]
          {
            nest_exceptions<std::logic_error>
              ( [&] { throw std::runtime_error (inner); }
              , outer
              );
          }
        , testing::make_nested
            (std::logic_error (outer), std::runtime_error (inner))
        );
    }
  }
}
