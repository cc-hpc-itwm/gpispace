// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/cxx17/holds_alternative.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/variant.hpp>

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      BOOST_AUTO_TEST_CASE (holds_queried_type)
      {
        ::boost::variant<int> const variant;
        BOOST_REQUIRE_EQUAL ( holds_alternative<int> (variant)
                            , true
                            );
      }

      BOOST_AUTO_TEST_CASE (does_not_hold_queried_type)
      {
        ::boost::variant<int> const variant;
        BOOST_REQUIRE_EQUAL ( holds_alternative<long> (variant)
                            , false
                            );
      }

      BOOST_AUTO_TEST_CASE (works_when_holding_multiple_types)
      {
        ::boost::variant<int, long> const variant;
        BOOST_REQUIRE_EQUAL ( holds_alternative<int> (variant)
                            , true
                            );
        BOOST_REQUIRE_EQUAL ( holds_alternative<long> (variant)
                            , false
                            );
      }

      BOOST_AUTO_TEST_CASE (works_after_assignment)
      {
        ::boost::variant<int, long> const  variant {2L};
        BOOST_REQUIRE_EQUAL ( holds_alternative<int> (variant)
                            , false
                            );
        BOOST_REQUIRE_EQUAL ( holds_alternative<long> (variant)
                            , true
                            );
      }
    }
  }
}
