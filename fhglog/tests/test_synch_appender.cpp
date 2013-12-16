// alexander.petry@itwm.fraunhofer.de

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/SynchronizedAppender.hpp>
#include <fhglog/NullAppender.hpp>

#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread.hpp>

#include <tests/utils.hpp>

namespace
{
  const std::size_t thread_count (100);
  const std::size_t message_count (1000);

  void worker (fhg::log::logger_t* logger)
  {
    for (std::size_t i (0); i < message_count; ++i)
    {
      logger->log (FHGLOG_MKEVENT_HERE (INFO, "hello"));
    }
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

  {
    boost::ptr_vector<boost::thread> threads;

    for (std::size_t i (0); i < thread_count; ++i)
    {
      threads.push_back (new boost::thread (worker, &log));
    }

    BOOST_FOREACH (boost::thread& thread, threads)
    {
      if (thread.joinable())
      {
        thread.join();
      }
    }
  }

  errcount += (messages_logged != (thread_count * message_count));

  return errcount;
}
