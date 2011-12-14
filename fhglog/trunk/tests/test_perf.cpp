/*
 * =====================================================================================
 *
 *       Filename:  test_perf.cpp
 *
 *    Description:  Tests the performance of appending serialized vs threaded
 *
 *        Version:  1.0
 *        Created:  09/18/2009 04:38:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/NullAppender.hpp>
#include <fhglog/ThreadedAppender.hpp>

#include <boost/thread/thread_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

int main (int , char **)
{
  using namespace fhg::log;

  int errcount(0);
  const std::size_t num_events(1000000);
  logger_t log(getLogger());

  {
    std::clog << "** testing serialized event appending performance...";

    Appender::ptr_t null(new NullAppender("null"));

	log.addAppender(null);

	const std::string msg("hello world!");

	boost::system_time start = boost::get_system_time();
	for (std::size_t cnt(0); cnt < num_events; ++cnt)
	{
	  log.log(FHGLOG_MKEVENT_HERE(DEBUG, msg));
	}
	boost::system_time end = boost::get_system_time();
	boost::posix_time::time_period tp(start, end);

	std::clog << "sequential logging took: " << tp.length() << std::endl;

	log.removeAllAppenders();
  }

  {
    std::clog << "** testing threaded event appending performance...";

	Appender::ptr_t null(new NullAppender("null"));

	ThreadedAppender::ptr_t threaded_appender(new ThreadedAppender(null));

	log.addAppender(threaded_appender);

	const std::string msg("hello world!");

	boost::system_time start = boost::get_system_time();
	for (std::size_t cnt(0); cnt < num_events; ++cnt)
	{
	  log.log(FHGLOG_MKEVENT_HERE(DEBUG, msg));
	}
	threaded_appender->flush();
	boost::system_time end = boost::get_system_time();
	boost::posix_time::time_period tp(start, end);

	std::clog << "threaded logging took " << tp.length() << std::endl;

	log.removeAllAppenders();
  }

  log.removeAllAppenders();
  return errcount;
}
