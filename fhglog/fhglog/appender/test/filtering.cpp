// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE filtering_appender
#include <boost/test/unit_test.hpp>

#include <fhglog/Filter.hpp>
#include <fhglog/appender/filtering.hpp>
#include <fhglog/appender/stream.hpp>
#include <fhglog/fhglog.hpp>

#include <sstream>

#include <tests/utils.hpp>

BOOST_FIXTURE_TEST_CASE (filter_levels_below, utils::logger_with_minimum_log_level)
{
  std::ostringstream logstream;

  fhg::log::FilteringAppender appender
    ( fhg::log::Appender::ptr_t (new fhg::log::StreamAppender (logstream, "%m"))
    , fhg::log::Filter::ptr_t (new fhg::log::LevelFilter (fhg::log::ERROR))
    );

  appender.append (FHGLOG_MKEVENT_HERE (DEBUG, "hello world!"));

  BOOST_REQUIRE (logstream.str().empty());

  appender.append (FHGLOG_MKEVENT_HERE (ERROR, "hello world!"));

  BOOST_REQUIRE_EQUAL (logstream.str(), "hello world!");
}
