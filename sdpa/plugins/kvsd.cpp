#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/util/bool.hpp>
#include <fhg/util/bool_io.hpp>
#include <fhg/util/threadname.hpp>

#include <boost/thread.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/peer_info.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>

class KeyValueStoreDaemon : FHG_PLUGIN
{
public:
  FHG_PLUGIN_START()
  {
    m_host = fhg_kernel()->get("host", "0");
    if (m_host == "*")
      m_host = "0";

    m_port = fhg_kernel()->get("port", "2439");
    m_reuse_address =
      fhg_kernel ()->get<fhg::util::bool_t> ("reuse_address", "true");
    m_store_path =
      fhg_kernel ()->get ("store", "");

    MLOG( INFO
        , "starting KeyValueStore @ [" << m_host << "]:" << m_port
        );

    m_io_pool = new fhg::com::io_service_pool (1);
    m_kvsd = new fhg::com::kvs::server::kvsd (m_store_path);
    m_server = new fhg::com::tcp_server ( *m_io_pool
                                        , *m_kvsd
                                        , m_host
                                        , m_port
                                        , m_reuse_address
                                        );
    try
    {
      m_server->start ();
    }
    catch (std::exception const &ex)
    {
      MLOG (ERROR, "could not start kvsd: " << ex.what ());

      FHG_PLUGIN_FAILED (EADDRINUSE);
    }

    m_thread = boost::thread (&fhg::com::io_service_pool::run, m_io_pool);
    fhg::util::set_threadname (m_thread, "[kvsd]");

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    m_io_pool->stop ();
    m_thread.join ();
    m_kvsd->save ();

    if (m_server) { delete m_server; m_server = 0; }
    if (m_kvsd) { delete m_kvsd; m_kvsd = 0; }
    if (m_io_pool) { delete m_io_pool; m_io_pool = 0; }

    FHG_PLUGIN_STOPPED();
  }

private:
  std::string m_host;
  std::string m_port;
  bool        m_reuse_address;
  std::string m_store_path;

  fhg::com::io_service_pool   *m_io_pool;
  fhg::com::kvs::server::kvsd *m_kvsd;
  fhg::com::tcp_server        *m_server;

  boost::thread m_thread;
};

EXPORT_FHG_PLUGIN( kvsd
                 , KeyValueStoreDaemon
                 , "kvsd"
                 , "provides a key value store"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.2"
                 , "NA"
                 , ""
                 , ""
                 );
