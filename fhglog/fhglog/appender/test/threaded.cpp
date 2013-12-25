// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE threaded
#include <boost/test/unit_test.hpp>

#include <fhglog/appender/stream.hpp>
#include <fhglog/appender/threaded.hpp>
#include <fhglog/fhglog.hpp>

#include <tests/utils.hpp>

#include <sstream>

BOOST_FIXTURE_TEST_CASE (threaded_appender, utils::logger_with_minimum_log_level)
{
  std::ostringstream logstream;

  fhg::log::ThreadedAppender appender
    (fhg::log::Appender::ptr_t (new fhg::log::StreamAppender (logstream, "%m")));

  appender.append (FHGLOG_MKEVENT_HERE (DEBUG, "hello world!"));
  appender.flush();

  BOOST_REQUIRE_EQUAL (logstream.str(), "hello world!");
}
