// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE append
#include <boost/test/unit_test.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhglog/appender/stream.hpp>

#include <sstream>

BOOST_AUTO_TEST_CASE (stream_appender)
{
  fhg::log::Logger::ptr_t log
    (fhg::log::Logger::get ("logger-stream_appender"));
  log->setLevel (fhg::log::TRACE);

  std::ostringstream logstream;
  log->addAppender (fhg::log::Appender::ptr_t (new fhg::log::StreamAppender (logstream, "%m")));

  log->log (FHGLOG_MKEVENT_HERE (TRACE, "hello world!"));

  BOOST_REQUIRE_EQUAL (logstream.str(), "hello world!");
}

BOOST_AUTO_TEST_CASE (two_stream_appenders)
{
  fhg::log::Logger::ptr_t log
    (fhg::log::Logger::get ("logger-two_stream_appenders"));
  log->setLevel (fhg::log::TRACE);

  std::ostringstream logstream_0;
  std::ostringstream logstream_1;

  log->addAppender (fhg::log::Appender::ptr_t (new fhg::log::StreamAppender (logstream_0, "%m")));
  log->addAppender (fhg::log::Appender::ptr_t (new fhg::log::StreamAppender (logstream_1, "%m")));

  log->log (FHGLOG_MKEVENT_HERE (TRACE, "hello world!"));

  BOOST_REQUIRE_EQUAL (logstream_0.str(), "hello world!");
  BOOST_REQUIRE_EQUAL (logstream_1.str(), "hello world!");
}
