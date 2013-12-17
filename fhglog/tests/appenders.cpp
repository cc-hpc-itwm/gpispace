// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE append
#include <boost/test/unit_test.hpp>

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/StreamAppender.hpp>
#include <fhglog/format.hpp>
#include <fhglog/NullAppender.hpp>

#include <tests/utils.hpp>

namespace
{
  using namespace fhg::log;

  class FormattingNullAppender : public Appender
  {
  public:
    FormattingNullAppender (const std::string &a_name, const std::string & fmt)
      : Appender (a_name)
      , fmt_ (fmt)
    {}

    void append (const LogEvent &evt)
    {
      format (fmt_, evt);
    }

    void flush () {}
  private:
    std::string fmt_;
  };
}

//! \todo These two should test by pushing into the logger before / after removing
BOOST_FIXTURE_TEST_CASE (NOTEST_add_and_remove_appender, utils::logger_with_minimum_log_level)
{
  const Appender::ptr_t appender (new NullAppender("null"));
  log.addAppender (appender);

  //! \todo Assert being added

  log.removeAppender ("null");

  //! \todo Assert being removed
}

BOOST_FIXTURE_TEST_CASE (NOTEST_add_and_remove_all_appenders, utils::logger_with_minimum_log_level)
{
  const Appender::ptr_t null_0 (new NullAppender("null-0"));
  const Appender::ptr_t null_1 (new NullAppender("null-1"));
  const Appender::ptr_t null_2 (new NullAppender("null-2"));
  const Appender::ptr_t null_3 (new NullAppender("null-3"));

  log.addAppender (null_0);
  log.addAppender (null_1);
  log.addAppender (null_2);
  log.addAppender (null_3);

  //! \todo Assert being added

  log.removeAllAppenders();

  //! \todo Assert being removed
}

//! \todo This is not a test!
BOOST_FIXTURE_TEST_CASE (NOTEST_formatting_performance, utils::logger_with_minimum_log_level)
{
  log.addAppender (Appender::ptr_t (new FormattingNullAppender ("null", default_format::LONG())));
  for (std::size_t count(0); count < 100000; ++count)
  {
    log.log (FHGLOG_MKEVENT_HERE (DEBUG, "hello world!"));
  }
}

BOOST_FIXTURE_TEST_CASE (stream_appender, utils::logger_with_minimum_log_level)
{
  std::ostringstream logstream;
  log.addAppender (Appender::ptr_t (new StreamAppender ("stringstream", logstream, "%m")));

  log.log (FHGLOG_MKEVENT_HERE (DEBUG, "hello world!"));

  BOOST_REQUIRE_EQUAL (logstream.str(), "hello world!");
}

BOOST_FIXTURE_TEST_CASE (two_stream_appenders, utils::logger_with_minimum_log_level)
{
  std::ostringstream logstream_0;
  std::ostringstream logstream_1;

  log.addAppender (Appender::ptr_t (new StreamAppender ("stringstream-0", logstream_0, "%m")));
  log.addAppender (Appender::ptr_t (new StreamAppender ("stringstream-1", logstream_1, "%m")));

  log.log (FHGLOG_MKEVENT_HERE (DEBUG, "hello world!"));

  BOOST_REQUIRE_EQUAL (logstream_0.str(), "hello world!");
  BOOST_REQUIRE_EQUAL (logstream_1.str(), "hello world!");
}
