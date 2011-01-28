/*
 * =====================================================================================
 *
 *       Filename:  test_synch_appender.cpp
 *
 *    Description:  Tests the synchronous appender for the fhglog logger
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
#include <fhglog/SynchronizedAppender.hpp>
#include <fhglog/NullAppender.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

class CountingAppender : public fhg::log::Appender
{
public:
  CountingAppender ()
    : fhg::log::Appender ("counter")
    , count (0)
  {}

  void append (const fhg::log::LogEvent &evt)
  {
    ++count;
  }

  void flush () {}

  std::size_t count;
};

struct thread_data_t
{
  std::size_t rank;
  std::size_t count;
  fhg::log::logger_t * logger;
};

void worker (thread_data_t * data)
{
  for (std::size_t i (0); i < data->count; ++i)
  {
    data->logger->log (FHGLOG_MKEVENT_HERE(INFO, "hello"));
  }
}

int main (int , char **)
{
  using namespace fhg::log;

  int errcount(0);
  logger_t log(getLogger());
  log.setLevel(LogLevel::MIN_LEVEL);

  CountingAppender *total_count (new CountingAppender());
  log.addAppender(Appender::ptr_t(new SynchronizedAppender(total_count)));

  const std::size_t thread_count (100);
  const std::size_t message_count (1000);

  typedef std::vector<boost::shared_ptr<boost::thread> > thread_list_t;
  typedef std::vector<thread_data_t> thread_data_list_t;

  thread_list_t threads;
  thread_data_list_t thread_data (thread_count);

  for (std::size_t i (0); i < thread_count; ++i)
  {
    thread_data[i].rank   = i;
    thread_data[i].logger = &log;
    thread_data[i].count  = message_count;
    threads.push_back
      (boost::shared_ptr<boost::thread>(new boost::thread (boost::bind (worker, &thread_data[i]))));
  }

  for (std::size_t i (0); i < thread_count; ++i)
  {
    threads[i]->join();
  }

  threads.clear();

  std::cout << "total count = " << total_count->count << std::endl;
  std::cout << "expected    = " << (thread_count * message_count) << std::endl;

  errcount += (total_count->count != (thread_count * message_count));

  return errcount;
}
