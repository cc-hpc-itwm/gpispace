// alexander.petry@itwm.fraunhofer.de

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/SynchronizedAppender.hpp>
#include <fhglog/NullAppender.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <tests/utils.hpp>

void worker (std::size_t count, fhg::log::logger_t* logger)
{
  for (std::size_t i (0); i < count; ++i)
  {
    logger->log (FHGLOG_MKEVENT_HERE (INFO, "hello"));
  }
}

int main (int , char **)
{
  using namespace fhg::log;

  int errcount(0);
  logger_t log(getLogger());
  log.setLevel(LogLevel::MIN_LEVEL);

  std::size_t messages_logged (0);
  log.addAppender(Appender::ptr_t(new SynchronizedAppender(new utils::counting_appender (&messages_logged))));

  const std::size_t thread_count (100);
  const std::size_t message_count (1000);

  typedef std::vector<boost::shared_ptr<boost::thread> >  threads;
  for (std::size_t i (0); i < thread_count; ++i)
  {
    threads.push_back
      (boost::shared_ptr<boost::thread>(new boost::thread (boost::bind (worker, message_count, &log))));
  }

  for (std::size_t i (0); i < thread_count; ++i)
  {
    threads[i]->join();
  }

  threads.clear();

  std::cout << "total count = " << messages_logged << std::endl;
  std::cout << "expected    = " << (thread_count * message_count) << std::endl;

  errcount += (messages_logged != (thread_count * message_count));

  return errcount;
}
