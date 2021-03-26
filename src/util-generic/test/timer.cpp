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

#include <util-generic/testing/random.hpp>
#include <util-generic/timer/scoped.hpp>
#include <util-generic/timer/sections.hpp>

#include <boost/test/unit_test.hpp>

#include <chrono>
#include <sstream>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace timer
    {
      namespace
      {
        std::string START (std::string description)
        {
          return "START: " + description + "...\n";
        }
        std::string DONE (std::string description)
        {
          return "DONE: " + description + " [0 s]\n";
        }
      }

      BOOST_AUTO_TEST_CASE (scoped_timer_emits_start_and_done_messages)
      {
        auto const description (testing::random<std::string>{}());

        std::ostringstream oss;

        scoped<std::chrono::seconds> (description, oss);

        BOOST_REQUIRE_EQUAL
          (oss.str(), START (description) + DONE (description));
      }

      BOOST_AUTO_TEST_CASE (sections_timer_emits_total)
      {
        auto const total (testing::random<std::string>{}());

        std::ostringstream oss;

        sections<std::chrono::seconds> (total, oss);

        BOOST_REQUIRE_EQUAL
          (oss.str(), START (total) + DONE (total));
      }

      BOOST_AUTO_TEST_CASE (sections_timer_can_start_and_end_sections)
      {
        auto const total (testing::random<std::string>{}());
        auto const section (testing::random<std::string>{}());

        std::ostringstream oss;

        {
          sections<std::chrono::seconds> sections (total, oss);

          sections.section (section);
          sections.end_section();
        }

        BOOST_REQUIRE_EQUAL
          ( oss.str()
          , START (total) + START (section) + DONE (section) + DONE (total)
          );
      }

      BOOST_AUTO_TEST_CASE (sections_timer_ends_section_when_destructed)
      {
        auto const total (testing::random<std::string>{}());
        auto const section (testing::random<std::string>{}());

        std::ostringstream oss;

        {
          sections<std::chrono::seconds> sections (total, oss);

          sections.section (section);
        }

        BOOST_REQUIRE_EQUAL
          ( oss.str()
          , START (total) + START (section) + DONE (section) + DONE (total)
          );
      }

      BOOST_AUTO_TEST_CASE (sections_timer_ends_section_when_new_section_starts)
      {
        auto const total (testing::random<std::string>{}());
        auto const section1 (testing::random<std::string>{}());
        auto const section2 (testing::random<std::string>{}());

        std::ostringstream oss;

        {
          sections<std::chrono::seconds> sections (total, oss);

          sections.section (section1);
          sections.section (section2);
        }

        BOOST_REQUIRE_EQUAL
          ( oss.str()
          , START (total) + START (section1) + DONE (section1)
                          + START (section2) + DONE (section2)
          + DONE (total)
          );
      }
    }
  }
}
