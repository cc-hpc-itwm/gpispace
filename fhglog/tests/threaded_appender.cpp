// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE threaded
#include <boost/test/unit_test.hpp>

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/StreamAppender.hpp>
#include <fhglog/ThreadedAppender.hpp>

#include <tests/utils.hpp>

BOOST_FIXTURE_TEST_CASE (threaded_appender, utils::logger_with_minimum_log_level)
{
  std::ostringstream logstream;

  fhg::log::ThreadedAppender appender
    (new fhg::log::StreamAppender ("s1", logstream, "%m"));

  appender.append (FHGLOG_MKEVENT_HERE (DEBUG, "hello world!"));
	appender.flush();

  BOOST_REQUIRE_EQUAL (logstream.str(), "hello world!");
}
