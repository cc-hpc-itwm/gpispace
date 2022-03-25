// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <util-generic/ostream/to_string.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      BOOST_AUTO_TEST_CASE (to_string_uses_id_by_default)
      {
        auto const value (testing::random<int>{}());
        BOOST_REQUIRE_EQUAL
          (ostream::to_string (value), std::to_string (value));
      }

      namespace
      {
        std::ostream& weird_int_printer (std::ostream& os, int const& i)
        {
          return os << "-" << -i;
        }
      }

      BOOST_AUTO_TEST_CASE (to_string_may_use_a_different_printer)
      {
        auto const value (testing::random<int>{}());
        BOOST_REQUIRE_EQUAL
          ( ostream::to_string
              (value, callback::print_function<int> (weird_int_printer))
          , "-" + std::to_string (-value)
          );
      }
    }
  }
}
