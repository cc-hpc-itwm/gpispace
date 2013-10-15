#define BOOST_TEST_MODULE fhglog_event
#include <boost/test/unit_test.hpp>

#include <fhglog/fhglog.hpp>

#include <sys/time.h>

namespace
{
  fhg::log::LogEvent gen_event()
  {
    fhg::log::LogEvent evt ( fhg::log::LogLevel::DEBUG
                           , __FILE__
                           , "main", __LINE__, "hello world!"
                           );

    evt.tag ("foo");
    evt.tag ("bar");

    evt.trace ("trace1\"trace1");
    evt.trace ("t2't2\"\"\"");

    return evt;
  }
}

BOOST_AUTO_TEST_CASE (encode_decode)
{
  const std::string evts (gen_event().encode());

  BOOST_REQUIRE_EQUAL (fhg::log::LogEvent::from_string (evts).encode(), evts);
}

namespace
{
  double current_time()
  {
    struct timeval tv;

    gettimeofday (&tv, NULL);

    return ((double)(tv.tv_sec) + (double)(tv.tv_usec) * 1e-6);
  }
}

BOOST_AUTO_TEST_CASE (encode_with_time_constraint)
{
  fhg::log::LogEvent evt (gen_event());
  const char first_encoded_char (*evt.encode().begin());

  const std::size_t max (250000);

  int count (0);

  double t (-current_time());

  for (std::size_t i (0); i < max; ++i)
  {
    count += *evt.encode().begin();
  }

  t += current_time();

  BOOST_REQUIRE_EQUAL (count, max * int (first_encoded_char));
  BOOST_REQUIRE_LT (t, 1.0);
}

BOOST_AUTO_TEST_CASE (decode_with_time_constraint)
{
  fhg::log::LogEvent evt (gen_event());
  const std::string evts (evt.encode());
  const size_t id (evt.id());

  const std::size_t max (750000);

  std::size_t count (0);

  double t (-current_time());

  for (std::size_t i (0); i < max; ++i)
  {
    count += fhg::log::LogEvent::from_string (evts).id();
  }

  t += current_time();

  BOOST_REQUIRE_EQUAL (count, max * id);
  BOOST_REQUIRE_LT (t, 1.0);
}
