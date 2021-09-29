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
        boost::variant<int> const variant;
        BOOST_REQUIRE_EQUAL ( holds_alternative<int> (variant)
                            , true
                            );
      }

      BOOST_AUTO_TEST_CASE (does_not_hold_queried_type)
      {
        boost::variant<int> const variant;
        BOOST_REQUIRE_EQUAL ( holds_alternative<long> (variant)
                            , false
                            );
      }

      BOOST_AUTO_TEST_CASE (works_when_holding_multiple_types)
      {
        boost::variant<int, long> const variant;
        BOOST_REQUIRE_EQUAL ( holds_alternative<int> (variant)
                            , true
                            );
        BOOST_REQUIRE_EQUAL ( holds_alternative<long> (variant)
                            , false
                            );
      }

      BOOST_AUTO_TEST_CASE (works_after_assignment)
      {
        boost::variant<int, long> const  variant {2L};
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
