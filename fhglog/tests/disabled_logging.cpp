// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE disabled_logging
#include <boost/test/unit_test.hpp>

#include <sstream> // ostringstream
#include <ctime>   // time

#define FHGLOG_DISABLE_LOGGING 1
#include <fhglog/fhglog.hpp>
#include <fhglog/StreamAppender.hpp>

#include <tests/utils.hpp>

BOOST_FIXTURE_TEST_CASE (logging_disabled_should_add_nothing_to_stream, utils::logger_with_minimum_log_level)
{
  std::size_t messages_logged (0);
  log.addAppender ( fhg::log::Appender::ptr_t
                    (new utils::counting_appender (&messages_logged))
                  );

  log.log (FHGLOG_MKEVENT_HERE (DEBUG, "hello world!"));

  BOOST_REQUIRE_EQUAL (messages_logged, 0);
}

namespace
{
  struct bad_timer
  {
    bad_timer()
      : _start (time (NULL))
    {}
    std::time_t elapsed() const
    {
      return time (NULL) - _start;
    }

  private:
    std::time_t _start;
  };
}

BOOST_FIXTURE_TEST_CASE (logging_disabled_should_use_less_than_a_second_and_not_assemble_message, utils::logger_with_minimum_log_level)
{
  const bad_timer timer;

  LLOG (TRACE, log, sleep (10));
  LLOG (INFO, log, sleep (10));
  LLOG (WARN, log, sleep (10));
  LLOG (ERROR, log, sleep (10));
  LLOG (FATAL, log, sleep (10));

  BOOST_REQUIRE_LE (timer.elapsed(), 1);
}