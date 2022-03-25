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

//! \note: that test might fail _to output_ differences as the output
//! stream might be redirected :(

#include <util-generic/ostream/redirect_standard_streams.hpp>
#include <util-generic/testing/printer/vector.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <iostream>
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

      BOOST_AUTO_TEST_CASE
        (redirect_standard_streams_pushes_all_to_same_vector_with_prefix)
      {
        std::vector<std::string> lines;
        std::vector<std::size_t> lines_counts;
        std::vector<std::string> expected;

        {
          redirect_standard_streams const redirecter (lines);

          for (int i (0); i < 10; ++i)
          {
            auto const line (testing::random<std::string>{} (any_single_line));
            std::ostream& os ( i % 3 == 0 ? std::clog
                             : i % 3 == 1 ? std::cout
                             : std::cerr
                             );

            os << line << '\n';

            expected.emplace_back (( i % 3 == 0 ? "log: "
                                   : i % 3 == 1 ? "out: "
                                   : "err: "
                                   ) + line
                                  );

            lines_counts.emplace_back (lines.size());
          }
        }

        BOOST_REQUIRE_EQUAL (lines, expected);

        std::vector<std::size_t> expected_counts (expected.size());
        std::iota (expected_counts.begin(), expected_counts.end(), 1);
        BOOST_REQUIRE_EQUAL (lines_counts, expected_counts);
      }
    }
  }
}
