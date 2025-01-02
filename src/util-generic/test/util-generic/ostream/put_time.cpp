// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/ostream/put_time.hpp>

#include <boost/test/unit_test.hpp>

#include <chrono>
#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      BOOST_AUTO_TEST_CASE (unix_epoch_printed)
      {
        struct tm helper;
        helper.tm_year = 70;
        helper.tm_mon = 0;
        helper.tm_mday = 1;
        helper.tm_hour = 0;
        helper.tm_min = 0;
        helper.tm_sec = 0;
        helper.tm_isdst = -1;
        std::time_t unix_epoch (std::mktime (&helper));
        auto const unix_epoch_timepoint
          (std::chrono::system_clock::from_time_t (unix_epoch));

        std::ostringstream oss;
        put_time<std::chrono::system_clock> {unix_epoch_timepoint} (oss);

        BOOST_REQUIRE_EQUAL ("1970-01-01 00:00:00", oss.str());
      }

      BOOST_AUTO_TEST_CASE (the_birth_of_our_lord_and_saviour_printed)
      {
        struct tm helper;
        helper.tm_year = 91;
        helper.tm_mon = 6;
        helper.tm_mday = 19;
        helper.tm_hour = 4;
        helper.tm_min = 30;
        helper.tm_sec = 18;
        helper.tm_isdst = -1;
        std::time_t the_birth_of_our_lord_and_saviour (std::mktime (&helper));
        auto const the_birth_of_our_lord_and_saviour_timepoint
          (std::chrono::system_clock::from_time_t (the_birth_of_our_lord_and_saviour));

        std::ostringstream oss;
        put_time<std::chrono::system_clock>
          {the_birth_of_our_lord_and_saviour_timepoint} (oss);

        BOOST_REQUIRE_EQUAL ("1991-07-19 04:30:18", oss.str());
      }
    }
  }
}
