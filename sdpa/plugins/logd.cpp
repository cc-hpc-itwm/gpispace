#include "observable.hpp"

#include <boost/function.hpp>

#include <fhglog/remote/LogServer.hpp>
#include <fhglog/Appender.hpp>
#include <fhglog/fhglog.hpp>
#include <fhg/plugin/plugin.hpp>

#include <fhg/util/threadname.hpp>

namespace detail
{
  class FunctionAppender : public fhg::log::Appender
  {
  public:
    typedef boost::function<void (const fhg::log::LogEvent&)> event_handler_t;

    FunctionAppender ( std::string const & n
                     , event_handler_t h
                     )
      : fhg::log::Appender(n)
      , m_handler (h)
    {}

    void append (fhg::log::LogEvent const &evt)
    {
      m_handler(evt);
    }

    void flush() {}
  private:
    event_handler_t m_handler;
    //    unsigned short m_port;
  };
}

typedef boost::shared_ptr<fhg::log::remote::LogServer> logserver_t;

class LogdPluginImpl : FHG_PLUGIN
                     , public observe::Observable
{
public:
  FHG_PLUGIN_START()
  {
    m_port = fhg_kernel()->get<unsigned short>("port", "3456");

    m_log_server = logserver_t
      (new fhg::log::remote::LogServer
      ( fhg::log::Appender::ptr_t (new detail::FunctionAppender
                                  ( "logd-appender"
                                  , boost::bind( &LogdPluginImpl::emit
                                               , this
                                               , _1
                                               )
                                  )
                                  )
      , m_io_service, m_port));

    m_io_thread = boost::thread
      (boost::bind (&boost::asio::io_service::run, &m_io_service));
    fhg::util::set_threadname (m_io_thread, "[logd]");

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    m_io_service.stop();
    m_io_thread.interrupt();
    m_io_thread.join();

    FHG_PLUGIN_STOPPED();
  }
private:
  unsigned short m_port;
  boost::asio::io_service m_io_service;
  boost::thread m_io_thread;
  logserver_t m_log_server;
};

EXPORT_FHG_PLUGIN( logd
                 , LogdPluginImpl
                 , ""
                 , "provides access to the logging server"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
