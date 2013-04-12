#include "kvs.hpp"

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/util/bool_io.hpp>

#include <fhgcom/kvs/kvsc.hpp>

class KeyValueStorePlugin : FHG_PLUGIN
                          , public kvs::KeyValueStore
{
public:
  KeyValueStorePlugin ()
    : m_host ("localhost")
    , m_port ("2439")
    , m_ping_interval (10)
    , m_ping_failed (0)
    , m_max_ping_failed (3)
    , m_kvs_timeout (120)
    , m_terminate_on_connection_failure (true)
  {}

  FHG_PLUGIN_START()
  {
    m_host = fhg_kernel()->get("host", m_host);
    m_port = fhg_kernel()->get("port", m_port);
    m_max_ping_failed = fhg_kernel ()->get ( "max_ping_failed"
                                           , m_max_ping_failed
                                           );
    m_ping_interval =
      fhg_kernel ()->get<unsigned int> ("ping", m_ping_interval);
    m_terminate_on_connection_failure =
      fhg_kernel ()->get<fhg::util::bool_t> ( "terminate"
                                            , m_terminate_on_connection_failure
                                            );
    m_kvs_timeout = fhg_kernel ()->get<unsigned int> ("timeout", m_kvs_timeout);

    DMLOG( TRACE
         , "initializing KeyValueStore @ [" << m_host << "]:" << m_port
         );

    fhg::com::kvs::global::get_kvs_info().init
      ( m_host
      , m_port
      , boost::posix_time::seconds (m_kvs_timeout)
      , 1
      );

    async_start();

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  value_type get(key_type const & k, value_type const &dflt) const
  {
    try
    {
      return fhg::com::kvs::get<std::string>(k);
    }
    catch (std::exception const & ex)
    {
      return dflt;
    }
  }

  void       put(key_type const & k, value_type const &value)
  {
    fhg::com::kvs::put(k,value);
  }

  void       del(key_type const & k)
  {
    fhg::com::kvs::del(k);
  }

  int        inc(key_type const & k, int step)
  {
    return fhg::com::kvs::inc(k, step);
  }

  bool       ping () const
  {
    try
    {
      return fhg::com::kvs::ping ();
    }
    catch (std::exception const &ex)
    {
      return false;
    }
  }
private:
  void async_start ()
  {
    try
    {
      fhg::com::kvs::global::get_kvs_info().start();

      if (m_ping_interval)
        fhg_kernel ()->schedule ( "kvs_ping"
                                , boost::bind ( &KeyValueStorePlugin::kvs_ping
                                              , this
                                              )
                                , m_ping_interval
                                );
    }
    catch (std::exception const & ex)
    {
      MLOG_EVERY_N (ERROR, 5, "could not connect to KVS: " << ex.what());
      MLOG_ONCE ( INFO
                , "HINT: make sure that the KVS is actually started and/or"
                << " check the plugin.kvs.host and plugin.kvs.port settings"
                );

      if (m_terminate_on_connection_failure)
      {
        fhg_kernel ()->shutdown ();
        return;
      }
      else
      {
        fhg_kernel()->schedule ( "kvs_connect"
                               , boost::bind ( &KeyValueStorePlugin::async_start
                                             , this
                                             )
                               , 10
                               );
      }
    }
  }

  void kvs_ping ()
  {
    if (! this->ping ())
    {
      ++m_ping_failed;

      if (m_ping_failed >= m_max_ping_failed)
      {
        if (m_terminate_on_connection_failure)
        {
          MLOG (WARN, "lost connection to KVS, terminating...");
          fhg_kernel ()->shutdown ();
          return;
        }
      }
    }
    else
    {
      m_ping_failed = 0;
    }

    fhg_kernel ()->schedule ( "kvs_ping"
                            , boost::bind ( &KeyValueStorePlugin::kvs_ping
                                          , this
                                          )
                            , m_ping_interval
                            );
  }

  std::string  m_host;
  std::string  m_port;
  unsigned int m_ping_interval;
  unsigned int m_ping_failed;
  unsigned int m_max_ping_failed;
  unsigned int m_kvs_timeout;
  bool         m_terminate_on_connection_failure;
};


EXPORT_FHG_PLUGIN( kvs
                 , KeyValueStorePlugin
                 , "kvs"
                 , "provides access to a key value store"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.2"
                 , "NA"
                 , ""
                 , ""
                 );
