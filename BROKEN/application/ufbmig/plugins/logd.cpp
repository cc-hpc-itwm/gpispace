#include "observable.hpp"

#include <boost/function.hpp>

#include <fhglog/appender/call.hpp>
#include <fhglog/remote/server.hpp>
#include <fhglog/Appender.hpp>
#include <fhglog/fhglog.hpp>
#include <fhg/plugin/plugin.hpp>

#include <fhg/util/threadname.hpp>

typedef boost::shared_ptr<fhg::log::remote::LogServer> logserver_t;

class LogdPluginImpl : FHG_PLUGIN
                     , public observe::Observable
{
public:
  FHG_PLUGIN_START()
  {
    m_port = fhg_kernel()->get<unsigned short>("port", "3456");

    fhg::log::Logger::ptr_t l (fhg::log::Logger::get ("logd"));

    l->addAppender
      ( fhg::log::Appender::ptr_t
        ( new fhg::log::appender::call (boost::bind ( &LogdPluginImpl::emit
                                                    , this
                                                    , _1
                                                    )
                                       )
        )
      );

    m_log_server = logserver_t
      (new fhg::log::remote::LogServer (l, m_io_service, m_port));

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
