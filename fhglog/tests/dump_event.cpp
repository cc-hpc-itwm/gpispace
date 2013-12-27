#define BOOST_TEST_MODULE fhglog_event
#include <boost/test/unit_test.hpp>

#include <fhglog/fhglog.hpp>

#include <fhg/util/now.hpp>

namespace
{
  fhg::log::LogEvent gen_event()
  {
    std::vector<std::string> tags;
    tags.push_back ("foo");
    tags.push_back ("bar");

    fhg::log::LogEvent evt ( fhg::log::DEBUG
                           , __FILE__
                           , "main", __LINE__, "hello world!"
                           , tags
                           );

    evt.trace ("trace1\"trace1");
    evt.trace ("t2't2\"\"\"");

    return evt;
  }
}

BOOST_AUTO_TEST_CASE (encode_decode)
{
  const std::string evts (gen_event().encoded());

  BOOST_REQUIRE_EQUAL (fhg::log::LogEvent::from_string (evts).encoded(), evts);
}

BOOST_AUTO_TEST_CASE (encode_with_time_constraint)
{
  fhg::log::LogEvent evt (gen_event());
  const char first_encoded_char (*evt.encoded().begin());

  const std::size_t max (250000);

  int count (0);

  double t (-fhg::util::now());

  for (std::size_t i (0); i < max; ++i)
  {
    count += *evt.encoded().begin();
  }

  t += fhg::util::now();

  BOOST_REQUIRE_EQUAL (count, max * int (first_encoded_char));
#ifndef NDEBUG
  BOOST_REQUIRE_LT (t, 5.0);
#else
  BOOST_REQUIRE_LT (t, 1.0);
#endif
}

BOOST_AUTO_TEST_CASE (decode_with_time_constraint)
{
  fhg::log::LogEvent evt (gen_event());
  const std::string evts (evt.encoded());
  const pid_t id (evt.tid());

  const std::size_t max (750000);

  pid_t count (0);

  double t (-fhg::util::now());

  for (std::size_t i (0); i < max; ++i)
  {
    count += fhg::log::LogEvent::from_string (evts).tid();
  }

  t += fhg::util::now();

  BOOST_REQUIRE_EQUAL (count, pid_t (max * id));
#ifndef NDEBUG
  BOOST_REQUIRE_LT (t, 5.0);
#else
  BOOST_REQUIRE_LT (t, 1.0);
#endif
}
