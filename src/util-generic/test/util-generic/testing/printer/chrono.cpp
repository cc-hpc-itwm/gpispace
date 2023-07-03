// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/printer/chrono.hpp>
#include <util-generic/testing/random.hpp>

#include <util-generic/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

#include <chrono>

#define REQUIRE_PRINTED_AS_NUMBER_WITH_SUFFIX(value_, suffix_, type_...)  \
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS                                     \
    (std::to_string (value_) + " " + (suffix_), std::chrono::type_ {value_})

#define DO_DURATION_TEST(suffix_, name_, type_...)                      \
  BOOST_AUTO_TEST_CASE (name_ ## _hardcoded_values)                     \
  {                                                                     \
    REQUIRE_PRINTED_AS_NUMBER_WITH_SUFFIX (0, suffix_, type_);          \
    REQUIRE_PRINTED_AS_NUMBER_WITH_SUFFIX (10, suffix_, type_);         \
    REQUIRE_PRINTED_AS_NUMBER_WITH_SUFFIX (115, suffix_, type_);        \
    REQUIRE_PRINTED_AS_NUMBER_WITH_SUFFIX (-6337, suffix_, type_);      \
    REQUIRE_PRINTED_AS_NUMBER_WITH_SUFFIX (-1, suffix_, type_);         \
  }                                                                     \
                                                                        \
  BOOST_AUTO_TEST_CASE (name_ ## _random_values)                        \
  {                                                                     \
    {                                                                   \
      auto const value                                                  \
        (fhg::util::testing::random<std::chrono::type_::rep>{}());      \
      REQUIRE_PRINTED_AS_NUMBER_WITH_SUFFIX (value, suffix_, type_);    \
    }                                                                   \
    {                                                                   \
      auto const value                                                  \
        (fhg::util::testing::random<std::chrono::type_::rep>{}());      \
      REQUIRE_PRINTED_AS_NUMBER_WITH_SUFFIX (value, suffix_, type_);    \
    }                                                                   \
    {                                                                   \
      auto const value                                                  \
        (fhg::util::testing::random<std::chrono::type_::rep>{}());      \
      REQUIRE_PRINTED_AS_NUMBER_WITH_SUFFIX (value, suffix_, type_);    \
    }                                                                   \
  }

DO_DURATION_TEST ("ns", nanoseconds, nanoseconds)
DO_DURATION_TEST ("µs", microseconds, microseconds)
DO_DURATION_TEST ("ms", milliseconds, milliseconds)
DO_DURATION_TEST ("s", seconds, seconds)
DO_DURATION_TEST ("min", minutes, minutes)
DO_DURATION_TEST ("hr", hours, hours)

DO_DURATION_TEST
  ("[1/100000000]s", shakes, duration<int, std::ratio<1, 100000000>>)
DO_DURATION_TEST ("[1/100]s", jiffies, duration<int, std::centi>)
DO_DURATION_TEST
  ("[756/625]s", microfortnights, duration<int, std::ratio<12096, 10000>>)
DO_DURATION_TEST
  ("[631/200]s", nanocenturies, duration<int, std::ratio<3155, 1000>>)
DO_DURATION_TEST ("[1800]s", half_an_hour, duration<int, std::ratio<1800, 1>>)

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

  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("1970-01-01 00:00:00", unix_epoch_timepoint);
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

  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("1991-07-19 04:30:18", the_birth_of_our_lord_and_saviour_timepoint);
}
