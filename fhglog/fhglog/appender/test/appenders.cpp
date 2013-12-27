// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE append
#include <boost/test/unit_test.hpp>

#include <tests/utils.hpp>

#include <fhglog/fhglog.hpp>
#include <fhglog/appender/stream.hpp>

#include <sstream>

BOOST_FIXTURE_TEST_CASE (stream_appender, utils::logger_with_minimum_log_level)
{
  std::ostringstream logstream;
  log->addAppender (fhg::log::Appender::ptr_t (new fhg::log::StreamAppender (logstream, "%m")));

  log->log (FHGLOG_MKEVENT_HERE (DEBUG, "hello world!"));

  BOOST_REQUIRE_EQUAL (logstream.str(), "hello world!");
}

BOOST_FIXTURE_TEST_CASE (two_stream_appenders, utils::logger_with_minimum_log_level)
{
  std::ostringstream logstream_0;
  std::ostringstream logstream_1;

  log->addAppender (fhg::log::Appender::ptr_t (new fhg::log::StreamAppender (logstream_0, "%m")));
  log->addAppender (fhg::log::Appender::ptr_t (new fhg::log::StreamAppender (logstream_1, "%m")));

  log->log (FHGLOG_MKEVENT_HERE (DEBUG, "hello world!"));

  BOOST_REQUIRE_EQUAL (logstream_0.str(), "hello world!");
  BOOST_REQUIRE_EQUAL (logstream_1.str(), "hello world!");
}
