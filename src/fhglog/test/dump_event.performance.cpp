#include <boost/test/unit_test.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhglog/test/dump_event.common.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/measure_average_time.hpp>
#include <util-generic/testing/printer/chrono.hpp>

BOOST_AUTO_TEST_CASE (encode_with_time_constraint)
{
  fhg::log::LogEvent evt (gen_event());
  const char first_encoded_char (*evt.encoded().begin());

  const std::size_t max (200000);

  int count (0);

  BOOST_REQUIRE_LT
    ( fhg::util::testing::measure_average_time<std::chrono::milliseconds>
      ( [&count, evt]()
        {
          count += *evt.encoded().begin();
        }
      , max
      )
     , std::chrono::milliseconds (5)
    );

  BOOST_REQUIRE_EQUAL (count, max * int (first_encoded_char));
}

BOOST_AUTO_TEST_CASE (decode_with_time_constraint)
{
  fhg::log::LogEvent evt (gen_event());
  const std::string evts (evt.encoded());
  const pid_t id (evt.tid());

  const std::size_t max (500000);

  pid_t count (0);

  BOOST_REQUIRE_LT
    ( fhg::util::testing::measure_average_time<std::chrono::milliseconds>
      ( [&count, evts]()
        {
          count += fhg::log::LogEvent::from_string (evts).tid();
        }
      , max
      )
    , std::chrono::milliseconds (2)
    );

  BOOST_REQUIRE_EQUAL (count, pid_t (max * id));
}
