/*
 * =====================================================================================
 *
 *       Filename:  test_remote_logging.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  10/19/2009 02:24:52 PM
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
#include <fhglog/StreamAppender.hpp>
#include <fhglog/CompoundAppender.hpp>
#include <fhglog/remote/RemoteAppender.hpp>
#include <fhglog/remote/LogServer.hpp>

boost::asio::io_service io_service;

class StopAppender : public fhg::log::Appender
{
public:
  StopAppender(boost::asio::io_service & s)
    : fhg::log::Appender("stop")
    , service_(s)
  {}

  void append(const fhg::log::LogEvent &)
  {
    service_.stop();
  }
  void flush () {}
  boost::asio::io_service & service_;
};

int main ()
{
  using namespace fhg::log;

  int errcount(0);

  std::string message("hello server!");
  std::string server(FHGLOG_DEFAULT_LOCATION);

  logger_t log(getLogger());
  logger_t rem(getLogger("remote"));

  log.addAppender(Appender::ptr_t(new StreamAppender("console", std::clog, "%m%n")));
  rem.addAppender(Appender::ptr_t(new remote::RemoteAppender("remote", server)));

  CompoundAppender::ptr_t compound(new CompoundAppender("compound"));

  std::ostringstream logstream;
  Appender::ptr_t appender
    (new StreamAppender( "stringstream"
                       , logstream
                       , "%m"
                       )
    );
  compound->addAppender (appender);
  compound->addAppender (Appender::ptr_t(new StopAppender (io_service)));

  fhg::log::shared_ptr<fhg::log::remote::LogServer> logd
    (new fhg::log::remote::LogServer ( compound
                                     , io_service
                                     , FHGLOG_DEFAULT_PORT
                                     )
    );
  {
    std::clog << "** testing remote logging...";
    rem.log(FHGLOG_MKEVENT_HERE(ERROR, message));
    io_service.run();
    const std::string msg (logstream.str());
    if (msg == "hello server!")
    {
      std::clog << "OK!" << std::endl;
    }
    else
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "   expected: \"hello server!\"" << std::endl;
      std::clog << "     actual: \"" << msg << "\"" << std::endl;
      ++errcount;
    }
  }

  {
    std::clog << "** testing remote appender with illegal host...";
    try
    {
      Appender::ptr_t remote_appender(new remote::RemoteAppender("remote", "unknown-host"));
      remote_appender->append (FHGLOG_MKEVENT_HERE(ERROR, "BUMMER!"));
      std::clog << "FAILED!" << std::endl;
      std::clog << "   expected exception!" << std::endl;
      ++errcount;
    }
    catch (std::exception const & ex)
    {
      std::clog << "OK!" << std::endl;
      std::clog << "    exception text: " << ex.what() << std::endl;
    }
  }

  return errcount;
}
