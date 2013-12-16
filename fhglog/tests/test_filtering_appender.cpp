// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE filtering_appender
#include <boost/test/unit_test.hpp>

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/StreamAppender.hpp>
#include <fhglog/FilteringAppender.hpp>
#include <fhglog/Filter.hpp>

#include <tests/utils.hpp>

BOOST_FIXTURE_TEST_CASE (filter_levels_below, utils::logger_with_minimum_log_level)
{
  std::ostringstream logstream;
  fhg::log::Appender::ptr_t streamAppender
    (new fhg::log::StreamAppender ("stream", logstream, "%m"));

  const fhg::log::Filter::ptr_t filter
    (new fhg::log::LevelFilter (fhg::log::LogLevel::ERROR));
  fhg::log::Appender::ptr_t appender
    (new fhg::log::FilteringAppender (streamAppender, filter));

  appender->append (FHGLOG_MKEVENT_HERE (DEBUG, "hello world!"));

  BOOST_REQUIRE (logstream.str().empty());

  appender->append (FHGLOG_MKEVENT_HERE (ERROR, "hello world!"));

  BOOST_REQUIRE_EQUAL (logstream.str(), "hello world!");
}
