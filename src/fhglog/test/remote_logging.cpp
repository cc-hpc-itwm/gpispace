// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE remote_logging
#include <boost/test/unit_test.hpp>

#include <sstream> // ostringstream
#include <fhglog/LogMacros.hpp>
#include <fhglog/appender/stream.hpp>
#include <fhglog/remote/appender.hpp>
#include <fhglog/remote/server.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/cxx14/make_unique.hpp>

#include <boost/thread/scoped_thread.hpp>

#include <functional>
#include <memory>

namespace
{
  class TestAppender : public fhg::log::Appender
  {
  public:
    TestAppender ( boost::asio::io_service& s
                 , std::unique_ptr<fhg::log::Appender> appender
                 )
      : _appender (std::move (appender))
      , service_ (s)
    {}

    virtual void append (const fhg::log::LogEvent& event) override
    {
      _appender->append (event);
      service_.stop();
    }

    virtual void flush() override
    {
      _appender->flush();
    }

  private:
    std::unique_ptr<fhg::log::Appender> _appender;
    boost::asio::io_service& service_;
  };
}

BOOST_AUTO_TEST_CASE (log_to_fake_remote_stream)
{
  std::ostringstream logstream;
  boost::asio::io_service io_service;

  fhg::log::Logger logger;
  logger.addAppender<TestAppender>
      ( io_service
      , fhg::util::cxx14::make_unique<fhg::log::StreamAppender> (logstream, "%m")
      );

  fhg::log::remote::LogServer logd (logger, io_service, 2438);

  {
    const boost::strict_scoped_thread<> service_thread
      ([&io_service] { io_service.run(); });

    boost::asio::io_service appender_io_service;

    fhg::log::remote::RemoteAppender appender
      ("localhost:2438", appender_io_service);

    appender.append (FHGLOG_MKEVENT_HERE (ERROR, "hello server!"));
  }

  BOOST_REQUIRE_EQUAL (logstream.str(), "hello server!");
}

BOOST_AUTO_TEST_CASE (throw_with_unknown_host)
{
  boost::asio::io_service appender_io_service;

  BOOST_REQUIRE_THROW
    ( fhg::log::remote::RemoteAppender ("unknown-host", appender_io_service)
    , std::runtime_error
    );
}