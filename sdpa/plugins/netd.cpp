#include "netd.hpp"
#include "kvs.hpp"

#include <fhglog/minimal.hpp>
#include <gspc/net.hpp>
#include <gspc/net/service/echo.hpp>

#include <fhg/plugin/plugin.hpp>

class DaemonImpl : FHG_PLUGIN
               , public fhg::net::Daemon
{
public:
  FHG_PLUGIN_START()
  {
    gspc::net::handle ("/service/echo", gspc::net::service::echo ());

    m_url = fhg_kernel()->get ("url", "tcp://*");

    m_server = gspc::net::serve (m_url, m_qmgr);

    m_listen_url = m_server->url ();

    MLOG (DEBUG, "listening on " << m_listen_url);

    kvs::KeyValueStore *kvs = fhg_kernel ()->acquire<kvs::KeyValueStore>("kvs");
    if (kvs)
    {
      try
      {
        kvs->put ( "gspc.net."
                 + fhg_kernel ()->get_name ()
                 + ".url"
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

    FHG_PLUGIN_STOPPED();
  }
private:
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
