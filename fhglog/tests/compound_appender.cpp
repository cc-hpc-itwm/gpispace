// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE compound
#include <boost/test/unit_test.hpp>

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/appender/stream.hpp>
#include <fhglog/appender/compound.hpp>

#include <tests/utils.hpp>

BOOST_FIXTURE_TEST_CASE (compound_appender, utils::logger_with_minimum_log_level)
{
  fhg::log::CompoundAppender compound;

  std::ostringstream logstream;
  compound.addAppender (fhg::log::Appender::ptr_t (new fhg::log::StreamAppender (logstream, "%m")));
  compound.addAppender (fhg::log::Appender::ptr_t (new fhg::log::StreamAppender (logstream, "%m")));

  compound.append (FHGLOG_MKEVENT_HERE (DEBUG, "hello world!"));

  BOOST_REQUIRE_EQUAL (logstream.str(), "hello world!hello world!");
}
