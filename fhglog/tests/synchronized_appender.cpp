// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE synchronized_appender
#include <boost/test/unit_test.hpp>

#include <fhglog/SynchronizedAppender.hpp>

#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread.hpp>

#include <tests/utils.hpp>

namespace
{
  const std::size_t thread_count (100);
  const std::size_t message_count (1000);

  void worker (fhg::log::SynchronizedAppender* appender)
  {
    for (std::size_t i (0); i < message_count; ++i)
    {
      appender->append (FHGLOG_MKEVENT_HERE (ERROR, "hello"));
    }
  }
}

BOOST_AUTO_TEST_CASE (synchronized_appender)
{
  std::size_t messages_logged (0);

  fhg::log::SynchronizedAppender appender
    (new utils::counting_appender (&messages_logged));

  {
    boost::ptr_vector<boost::thread> threads;

    for (std::size_t i (0); i < thread_count; ++i)
    {
      threads.push_back (new boost::thread (&worker, &appender));
    }

    BOOST_FOREACH (boost::thread& thread, threads)
    {
      if (thread.joinable())
      {
        thread.join();
      }
    }
  }

  BOOST_REQUIRE_EQUAL (messages_logged, thread_count * message_count);
}
