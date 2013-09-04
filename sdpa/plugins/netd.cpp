#include "netd.hpp"
#include "kvs.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <fhglog/minimal.hpp>

#include <fhglog/format.hpp>
#include <fhglog/MemoryAppender.hpp>

#include <gspc/net.hpp>
#include <gspc/net/service/echo.hpp>

#include <fhg/plugin/plugin.hpp>

class DaemonImpl : FHG_PLUGIN
               , public fhg::net::Daemon
{
public:
  FHG_PLUGIN_START()
  {
    size_t nthreads = fhg_kernel ()->get ("nthreads", 4L);

    gspc::net::initialize (nthreads);

    gspc::net::handle ("/service/echo", gspc::net::service::echo ());

    gspc::net::handle
      ( "/service/backlog"
      , boost::bind (&DaemonImpl::service_backlog, this, _1, _2, _3)
      );

    m_url = fhg_kernel()->get ("url", "tcp://*");

    m_server = gspc::net::serve (m_url, m_qmgr);

    m_listen_url = m_server->url ();

    MLOG (DEBUG, "listening on " << m_listen_url);

    kvs::KeyValueStore *kvs = fhg_kernel ()->acquire<kvs::KeyValueStore>("kvs");
    if (kvs)
    {
      try
      {
        kvs->put ( "gspc.net.url."
                 + fhg_kernel ()->get_name ()
                 , m_listen_url
                 );
      }
      catch (std::exception const &ex)
      {
        MLOG (WARN, "could not store url to KVS: " << ex.what ());
      }

      fhg_kernel ()->release ("kvs");
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    if (m_server)
    {
      m_server->stop ();
    }

    gspc::net::shutdown ();

    FHG_PLUGIN_STOPPED();
  }
private:
  void service_backlog ( std::string const &dst
                       , gspc::net::frame const &rqst
                       , gspc::net::user_ptr user
                       )
  {
    gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

    fhg::log::MemoryAppender::backlog_t backlog =
      fhg::log::global_memory_appender ("system")->backlog ();

    typedef std::vector <fhg::log::LogEvent> event_list_t;
    event_list_t events;
    events.assign (backlog.rbegin (), backlog.rend ());

    std::ostringstream oss;
    BOOST_FOREACH (fhg::log::LogEvent const &evt, events)
    {
      oss << evt;
    }
    rply.set_body (oss.str ());

    user->deliver (rply);
  }

  std::string             m_url;
  std::string             m_listen_url;

  gspc::net::server::queue_manager_t m_qmgr;
  gspc::net::server_ptr_t m_server;
};

EXPORT_FHG_PLUGIN( netd
                 , DaemonImpl
                 , "netd"
                 , "provides access to the gspcnet infrastructure"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
