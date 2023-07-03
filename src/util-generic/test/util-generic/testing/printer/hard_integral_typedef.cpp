// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/hard_integral_typedef.hpp>
#include <util-generic/testing/printer/hard_integral_typedef.hpp>
#include <util-generic/testing/random.hpp>

#include <util-generic/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

FHG_UTIL_HARD_INTEGRAL_TYPEDEF (signed_t, int);
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_LOG_VALUE_PRINTER (signed_t)

FHG_UTIL_HARD_INTEGRAL_TYPEDEF (unsigned_t, unsigned int);
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_LOG_VALUE_PRINTER (unsigned_t)

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      BOOST_AUTO_TEST_CASE (prints_the_same_as_underlying_type_signed)
      {
        auto const value (random<signed_t::underlying_type>{}());

        FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
          (std::to_string (value), signed_t (value));
        FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
          (std::to_string (-value), signed_t (-value));
      }

      BOOST_AUTO_TEST_CASE (prints_the_same_as_underlying_type_unsigned)
      {
        auto const value (random<unsigned_t::underlying_type>{}());

        FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
          (std::to_string (value), unsigned_t (value));
      }
    }
  }
}
