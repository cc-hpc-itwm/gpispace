// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE synchronized_appender
#include <boost/test/unit_test.hpp>

#include <fhglog/appender/synchronized.hpp>

#include <fhglog/LogMacros.hpp>

#include <boost/foreach.hpp>
#include <boost/thread.hpp>

namespace
{
  class counting_appender : public fhg::log::Appender
  {
  public:
    counting_appender ( std::size_t* counter
                      , std::string const& message
                      )
      : _counter (counter)
      , _message (message)
    {}

    void append (const fhg::log::LogEvent &evt)
    {
      BOOST_REQUIRE_EQUAL (evt.message(), _message);

      ++(*_counter);
    }

    void flush () {}

  private:
    std::size_t* _counter;
    std::string const _message;
  };

  const std::size_t thread_count (100);
  const std::size_t message_count (1000);

  void worker ( fhg::log::LogEvent const& event
              , fhg::log::SynchronizedAppender* appender
              )
  {
    for (std::size_t i (0); i < message_count; ++i)
    {
      appender->append (event);
    }
  }
}

BOOST_AUTO_TEST_CASE (synchronized_appender)
{
  std::size_t messages_logged (0);
  std::string const message ("boo fazzi");

  fhg::log::SynchronizedAppender appender
    (fhg::log::Appender::ptr_t (new counting_appender ( &messages_logged
                                                      , message
                                                      )
                               )
    );

  fhg::log::LogEvent const event (FHGLOG_MKEVENT_HERE (ERROR, message));

  {
    boost::thread_group threads;

    for (std::size_t i (0); i < thread_count; ++i)
    {
      threads.create_thread (boost::bind (&worker, event, &appender));
    }

    threads.join_all();
  }

  BOOST_REQUIRE_EQUAL (messages_logged, thread_count * message_count);
}
