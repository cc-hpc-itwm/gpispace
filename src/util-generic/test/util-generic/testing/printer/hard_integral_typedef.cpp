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
