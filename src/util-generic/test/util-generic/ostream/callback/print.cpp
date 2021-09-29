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

#include <util-generic/ostream/callback/print.hpp>
#include <util-generic/testing/printer/vector.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      namespace callback
      {
        namespace
        {
          struct : std::ostream {} void_ostream;
        }

        BOOST_AUTO_TEST_CASE (prints_given_value_with_given_function)
        {
          auto value (testing::random<int>{}());
          auto const orig_value (value);

          std::vector<int> called_with;
          print<int> const p
            { [&] (std::ostream& os, int const& x) -> std::ostream&
              {
                called_with.emplace_back (x);
                return os;
              }
            , value
            };

          //! \note ensure it is captured by copy
          ++value;

          BOOST_REQUIRE_EQUAL (called_with, std::vector<int> {});
          p (void_ostream);
          BOOST_REQUIRE_EQUAL (called_with, std::vector<int> {orig_value});
          p (void_ostream);
          BOOST_REQUIRE_EQUAL
            (called_with, (std::vector<int> {orig_value, orig_value}));
        }
      }
    }
  }
}
