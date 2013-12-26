// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE remote_logging
#include <boost/test/unit_test.hpp>

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/appender/stream.hpp>
#include <fhglog/remote/appender.hpp>
#include <fhglog/remote/LogServer.hpp>

namespace
{
  class TestAppender : public fhg::log::Appender
  {
  public:
    TestAppender ( boost::asio::io_service& s
                 , fhg::log::Appender::ptr_t appender
                 )
      : _appender (appender)
      , service_ (s)
    {}

    void append (const fhg::log::LogEvent& event)
    {
      _appender->append (event);
      service_.stop();
    }

    void flush()
    {
      _appender->flush();
    }

  private:
    fhg::log::Appender::ptr_t _appender;
    boost::asio::io_service& service_;
  };
}

BOOST_AUTO_TEST_CASE (log_to_fake_remote_stream)
{
  std::ostringstream logstream;
  boost::asio::io_service io_service;

  boost::shared_ptr<TestAppender>
    test_appender
    ( new TestAppender
      ( io_service
      , fhg::log::Appender::ptr_t (new fhg::log::StreamAppender (logstream, "%m"))
      )
    );

  boost::shared_ptr<fhg::log::remote::LogServer> logd
    (new fhg::log::remote::LogServer (test_appender, io_service, FHGLOG_DEFAULT_PORT));

  boost::thread service_thread
    (boost::bind (&boost::asio::io_service::run, &io_service));

  fhg::log::remote::RemoteAppender appender (FHGLOG_DEFAULT_LOCATION);

  appender.append (FHGLOG_MKEVENT_HERE (ERROR, "hello server!"));

  if (service_thread.joinable())
  {
    service_thread.join();
  }

  BOOST_REQUIRE_EQUAL (logstream.str(), "hello server!");
}

BOOST_AUTO_TEST_CASE (throw_with_unknown_host)
{
  BOOST_REQUIRE_THROW
    (fhg::log::remote::RemoteAppender ("unknown-host"), std::runtime_error);
}
