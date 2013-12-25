// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE performance
#include <boost/test/unit_test.hpp>

#include <fhglog/fhglog.hpp>
#include <fhglog/appender/null.hpp>
#include <fhglog/appender/threaded.hpp>

#include <boost/thread/thread_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <tests/utils.hpp>

namespace
{
  const std::size_t num_events(1000000);
}

//! \todo Does not test!
BOOST_FIXTURE_TEST_CASE (NOTEST_sequential_appending, utils::logger_with_minimum_log_level)
{
  log.addAppender
    (fhg::log::Appender::ptr_t (new fhg::log::NullAppender));

  const std::string msg ("hello world!");

  const boost::system_time start (boost::get_system_time());
  for (std::size_t cnt(0); cnt < num_events; ++cnt)
  {
    log.log (FHGLOG_MKEVENT_HERE (DEBUG, msg));
  }
  const boost::system_time end (boost::get_system_time());
  const boost::posix_time::time_period tp (start, end);
}

//! \todo Does not test!
BOOST_FIXTURE_TEST_CASE (NOTEST_threaded_appending, utils::logger_with_minimum_log_level)
{
  fhg::log::ThreadedAppender::ptr_t threaded_appender
    (new fhg::log::ThreadedAppender (fhg::log::Appender::ptr_t (new fhg::log::NullAppender)));

  log.addAppender (threaded_appender);

  const std::string msg ("hello world!");

  const boost::system_time start (boost::get_system_time());
  for (std::size_t cnt(0); cnt < num_events; ++cnt)
  {
    log.log (FHGLOG_MKEVENT_HERE (DEBUG, msg));
  }
  threaded_appender->flush();
  const boost::system_time end (boost::get_system_time());
  const boost::posix_time::time_period tp (start, end);
}
