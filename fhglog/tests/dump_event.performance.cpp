#define BOOST_TEST_MODULE fhglog_event_performance
#include <boost/test/unit_test.hpp>

#include <fhglog/LogMacros.hpp>
#include <tests/dump_event.common.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>
#include <fhg/util/now.hpp>

BOOST_AUTO_TEST_CASE (encode_with_time_constraint)
{
  fhg::log::LogEvent evt (gen_event());
  const char first_encoded_char (*evt.encoded().begin());

  const std::size_t max (200000);

  int count (0);

  double t (-fhg::util::now());

  for (std::size_t i (0); i < max; ++i)
  {
    count += *evt.encoded().begin();
  }

  t += fhg::util::now();

  BOOST_REQUIRE_EQUAL (count, max * int (first_encoded_char));
  BOOST_REQUIRE_LT (t, 1.0);
}

BOOST_AUTO_TEST_CASE (decode_with_time_constraint)
{
  fhg::log::LogEvent evt (gen_event());
  const std::string evts (evt.encoded());
  const pid_t id (evt.tid());

  const std::size_t max (500000);

  pid_t count (0);

  double t (-fhg::util::now());

  for (std::size_t i (0); i < max; ++i)
  {
    count += fhg::log::LogEvent::from_string (evts).tid();
  }

  t += fhg::util::now();

  BOOST_REQUIRE_EQUAL (count, pid_t (max * id));
  BOOST_REQUIRE_LT (t, 1.0);
}
