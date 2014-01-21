#include "kvs.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <fhglog/LogMacros.hpp>

#include <fhglog/format.hpp>

#include <gspc/net/server/default_queue_manager.hpp>
#include <gspc/net/server/default_service_demux.hpp>
#include <gspc/net/io.hpp>
#include <gspc/net/serve.hpp>
#include <gspc/net/server.hpp>
#include <gspc/net/service/echo.hpp>

#include <fhg/plugin/plugin.hpp>

class DaemonImpl : FHG_PLUGIN
{
public:
  FHG_PLUGIN_START()
  {
    size_t nthreads = fhg_kernel ()->get ("nthreads", 4L);

    gspc::net::initialize (nthreads);

    gspc::net::server::default_service_demux().handle ("/service/echo", gspc::net::service::echo ());

    const std::string netd_url (fhg_kernel()->get ("url", "tcp://*"));

    m_server = gspc::net::serve (netd_url, gspc::net::server::default_queue_manager());

    const std::string listen_url (m_server->url());

    MLOG (DEBUG, "listening on " << listen_url);

    kvs::KeyValueStore *kvs = fhg_kernel ()->acquire<kvs::KeyValueStore>("kvs");
    if (kvs)
    {
      try
      {
        kvs->put ( "gspc.net.url."
                 + fhg_kernel ()->get_name ()
                 , listen_url
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
