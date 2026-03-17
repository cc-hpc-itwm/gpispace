#include <gspc/testing/random.hpp>
#include <gspc/util/timer/scoped.hpp>
#include <gspc/util/timer/sections.hpp>

#include <boost/test/unit_test.hpp>

#include <chrono>
#include <sstream>
#include <string>
#include <utility>



    namespace gspc::util::timer
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
        auto const description (gspc::testing::random<std::string>{}());

        std::ostringstream oss;

        std::ignore = scoped<std::chrono::seconds> (description, oss);

        BOOST_REQUIRE_EQUAL
          (oss.str(), START (description) + DONE (description));
      }

      BOOST_AUTO_TEST_CASE (sections_timer_emits_total)
      {
        auto const total (gspc::testing::random<std::string>{}());

        std::ostringstream oss;

        std::ignore = sections<std::chrono::seconds> (total, oss);

        BOOST_REQUIRE_EQUAL
          (oss.str(), START (total) + DONE (total));
      }

      BOOST_AUTO_TEST_CASE (sections_timer_can_start_and_end_sections)
      {
        auto const total (gspc::testing::random<std::string>{}());
        auto const section (gspc::testing::random<std::string>{}());

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
        auto const total (gspc::testing::random<std::string>{}());
        auto const section (gspc::testing::random<std::string>{}());

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
        auto const total (gspc::testing::random<std::string>{}());
        auto const section1 (gspc::testing::random<std::string>{}());
        auto const section2 (gspc::testing::random<std::string>{}());

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
