// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE append
#include <boost/test/unit_test.hpp>

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/appender/stream.hpp>
#include <fhglog/appender/null.hpp>

#include <tests/utils.hpp>

namespace
{
  using namespace fhg::log;
}

//! \todo These two should test by pushing into the logger before / after removing
BOOST_FIXTURE_TEST_CASE (NOTEST_add_and_remove_appender, utils::logger_with_minimum_log_level)
{
  const Appender::ptr_t appender (new NullAppender);
  log.addAppender (appender);

  //! \todo Assert being added

  log.removeAppender (appender);

  //! \todo Assert being removed
}

BOOST_FIXTURE_TEST_CASE (NOTEST_add_and_remove_all_appenders, utils::logger_with_minimum_log_level)
{
  const Appender::ptr_t null_0 (new NullAppender);
  const Appender::ptr_t null_1 (new NullAppender);
  const Appender::ptr_t null_2 (new NullAppender);
  const Appender::ptr_t null_3 (new NullAppender);

  log.addAppender (null_0);
  log.addAppender (null_1);
  log.addAppender (null_2);
  log.addAppender (null_3);

  //! \todo Assert being added

  log.removeAllAppenders();

  //! \todo Assert being removed
}

BOOST_FIXTURE_TEST_CASE (stream_appender, utils::logger_with_minimum_log_level)
{
  std::ostringstream logstream;
  log.addAppender (Appender::ptr_t (new StreamAppender (logstream, "%m")));

  log.log (FHGLOG_MKEVENT_HERE (DEBUG, "hello world!"));

  BOOST_REQUIRE_EQUAL (logstream.str(), "hello world!");
}

BOOST_FIXTURE_TEST_CASE (two_stream_appenders, utils::logger_with_minimum_log_level)
{
  std::ostringstream logstream_0;
  std::ostringstream logstream_1;

  log.addAppender (Appender::ptr_t (new StreamAppender (logstream_0, "%m")));
  log.addAppender (Appender::ptr_t (new StreamAppender (logstream_1, "%m")));

  log.log (FHGLOG_MKEVENT_HERE (DEBUG, "hello world!"));

  BOOST_REQUIRE_EQUAL (logstream_0.str(), "hello world!");
  BOOST_REQUIRE_EQUAL (logstream_1.str(), "hello world!");
}
