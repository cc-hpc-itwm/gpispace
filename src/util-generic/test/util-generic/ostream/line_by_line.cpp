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

#include <util-generic/ostream/line_by_line.hpp>
#include <util-generic/testing/printer/vector.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      BOOST_AUTO_TEST_CASE (callback_is_called_with_remaining_data_in_dtor)
      {
        std::vector<std::string> lines;
        {
          line_by_line buffer
            ([&] (std::string const& s) { lines.emplace_back (s); });
          std::stringstream ss;
          static_cast<std::ostream&> (ss).rdbuf (&buffer);

          ss << "no newline ever";

          BOOST_REQUIRE (lines.empty());
        }
        BOOST_REQUIRE_EQUAL
          (lines, std::vector<std::string> {"no newline ever"});
      }

      BOOST_AUTO_TEST_CASE (callback_is_called_on_every_newline)
      {
        std::vector<std::string> lines;

        line_by_line buffer
          ([&] (std::string const& s) { lines.emplace_back (s); });
        std::stringstream ss;
        static_cast<std::ostream&> (ss).rdbuf (&buffer);

        ss << "first newline\nright in the middle";

        BOOST_REQUIRE_EQUAL (lines, std::vector<std::string> {"first newline"});

        ss << "\n";

        BOOST_REQUIRE_EQUAL
          ( lines
          , (std::vector<std::string> {"first newline", "right in the middle"})
          );
      }
    }
  }
}
