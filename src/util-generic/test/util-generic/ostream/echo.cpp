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

#include <util-generic/ostream/echo.hpp>

#include <boost/test/unit_test.hpp>

#include <chrono>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      namespace
      {
        //! \note we can't exactly verify that we have a timestamp in
        //! front because it uses system time which is obviously not
        //! easily determined in the same second. We'll be fine with
        //! just the opening braces and some numbers in between
        //! starting with the right year at least. We also know the
        //! exact length of a single date.

        std::size_t require_line_at
          (std::string const& str, std::size_t pos, std::string const& line)
        {
          auto const tt ( std::chrono::system_clock::to_time_t
                            (std::chrono::system_clock::now())
                        );
          auto const prefix
            ("[" + std::to_string (localtime (&tt)->tm_year + 1900));
          auto const end_of_time (pos + 1 + strlen ("YYYY-MM-DD hh:mm:ss"));

          BOOST_REQUIRE_EQUAL (str.substr (pos, prefix.size()), prefix);
          BOOST_REQUIRE_EQUAL (str.substr (end_of_time, 2), "] ");
          BOOST_REQUIRE_EQUAL (str.substr (end_of_time + 2, line.size()), line);

          return end_of_time + 2 + line.size();
        }
      }

      BOOST_AUTO_TEST_CASE
        (echo_prepends_time_in_braces_to_every_line_and_forwards_to_stream)
      {
        std::ostringstream oss;

        echo echoer (oss);

        std::string const first_line ("first\n");
        std::string const second_line ("second and\n");
        std::string const third_line (" third line\n");

        echoer << first_line;

        auto const end_of_first_line
          (require_line_at (oss.str(), 0, first_line));
        BOOST_REQUIRE_EQUAL (end_of_first_line, oss.str().size());

        echoer << (second_line + third_line);

        auto const end_of_second_line
          (require_line_at (oss.str(), end_of_first_line, second_line));
        require_line_at (oss.str(), end_of_second_line, third_line);
      }
    }
  }
}
