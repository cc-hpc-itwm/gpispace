// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
