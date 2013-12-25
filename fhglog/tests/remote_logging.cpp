// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE remote_logging
#include <boost/test/unit_test.hpp>

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/StreamAppender.hpp>
#include <fhglog/CompoundAppender.hpp>
#include <fhglog/remote/RemoteAppender.hpp>
#include <fhglog/remote/LogServer.hpp>

namespace
{
  class StopAppender : public fhg::log::Appender
  {
  public:
    StopAppender(boost::asio::io_service & s)
      : service_(s)
    {}

    void append(const fhg::log::LogEvent &)
    {
      service_.stop();
    }
    void flush () {}
    boost::asio::io_service & service_;
  };
}

BOOST_AUTO_TEST_CASE (log_to_fake_remote_stream)
{
  fhg::log::CompoundAppender::ptr_t compound (new fhg::log::CompoundAppender);

  std::ostringstream logstream;
  compound->addAppender
    (fhg::log::Appender::ptr_t (new fhg::log::StreamAppender (logstream, "%m")));

  boost::asio::io_service io_service;
  compound->addAppender
    (fhg::log::Appender::ptr_t (new StopAppender (io_service)));
  boost::shared_ptr<fhg::log::remote::LogServer> logd
    (new fhg::log::remote::LogServer (compound, io_service, FHGLOG_DEFAULT_PORT));

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
