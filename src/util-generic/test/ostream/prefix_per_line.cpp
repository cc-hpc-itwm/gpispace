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

#include <boost/test/unit_test.hpp>

#include <util-generic/ostream/prefix_per_line.hpp>
#include <util-generic/testing/random.hpp>

#include <sstream>
#include <string>
#include <vector>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      namespace
      {
        std::string const any_single_line
          {testing::random<std::string>::except ("\n")};
      }

      BOOST_AUTO_TEST_CASE (each_line_gets_the_prefix)
      {
        using fhg::util::testing::random;

        std::ostringstream oss;
        std::ostringstream expected;

        auto const prefix (random<std::string>{}());

        prefix_per_line _ (prefix, oss);

        auto n (random<std::size_t>{} (1000));

        while (n --> 0)
        {
          auto const line (random<std::string>{} (any_single_line));

          expected << prefix << line << '\n';
          _ << line << '\n';
        }

        BOOST_REQUIRE_EQUAL (expected.str(), oss.str());
      }
    }
  }
}
