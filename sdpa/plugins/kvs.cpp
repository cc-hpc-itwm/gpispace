#include "kvs.hpp"

#include <fhglog/LogMacros.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/util/read_bool.hpp>

#include <fhgcom/kvs/kvsc.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

class KeyValueStorePlugin : FHG_PLUGIN
                          , public kvs::KeyValueStore
{
public:
  KeyValueStorePlugin ()
    : m_ping_interval (boost::posix_time::seconds (5))
    , m_ping_failed (0)
    , m_max_ping_failed (3)
  {}

  FHG_PLUGIN_START()
  try
  {
    _request_stop = boost::bind (&fhg::plugin::Kernel::shutdown, fhg_kernel());

    std::string  m_host ("localhost");
    m_host = fhg_kernel()->get("host", m_host);
    std::string  m_port ("2439");
    m_port = fhg_kernel()->get("port", m_port);

    if (m_host.empty())
    {
      throw std::runtime_error ("kvs host empty");
    }
    if (m_port.empty())
    {
      throw std::runtime_error ("kvs port empty");
    }

    m_max_ping_failed = fhg_kernel ()->get ( "max_ping_failed"
                                           , m_max_ping_failed
                                           );
    m_ping_interval =
      boost::posix_time::duration_from_string
        ( fhg_kernel ()->get<std::string>
          ("ping", boost::posix_time::to_simple_string (m_ping_interval))
        );

    unsigned int m_kvs_timeout (120);
    m_kvs_timeout = fhg_kernel ()->get<unsigned int> ("timeout", m_kvs_timeout);

    _kvs_client = new fhg::com::kvs::client::kvsc
      ( m_host , m_port
      , true // auto_reconnect
      , boost::posix_time::seconds (m_kvs_timeout)
      , 1 // max_connection_attempts
      );

    _ping_thread = new boost::thread (&KeyValueStorePlugin::kvs_ping, this);

    fhg::com::kvs::get_or_create_global_kvs
      (m_host, m_port, true, boost::posix_time::seconds (m_kvs_timeout), 1);

    FHG_PLUGIN_STARTED();
  }
  catch (std::exception const & ex)
  {
    MLOG (ERROR, "could not connect to KVS: " << ex.what()
              << "HINT: make sure that the KVS is actually started and/or"
              << " check the plugin.kvs.host and plugin.kvs.port settings"
         );

    FHG_PLUGIN_FAILED (-1);
  }

  FHG_PLUGIN_STOP()
  {
    _ping_thread->interrupt();
    if (_ping_thread->joinable())
    {
      _ping_thread->join();
    }
    delete _ping_thread;
    _ping_thread = NULL;

    delete _kvs_client;
    _kvs_client = NULL;

    FHG_PLUGIN_STOPPED();
  }

  value_type get(key_type const & k, value_type const &dflt) const
  {
    try
    {
      key_value_map_type v (list (k));
      if (v.size() == 1)
      {
        return boost::lexical_cast<value_type>(v.begin()->second);
      }
      else
      {
        throw std::runtime_error("kvs::get: returned 0 or more than 1 element");
      }
    }
    catch (std::exception const & ex)
    {
      return dflt;
    }
  }

  void       put(key_type const & k, value_type const &value)
  {
    _kvs_client->put(k,value);
  }

  void       del(key_type const & k)
  {
    _kvs_client->del(k);
  }

  int        inc(key_type const & k, int step)
  {
    return _kvs_client->inc(k, step);
  }

  key_value_map_type list () const
  {
    return this->list ("");
  }

  key_value_map_type list (key_type const &prefix) const
  {
    return _kvs_client->get (prefix);
  }

  bool       ping () const
  {
    try
    {
      return _kvs_client->ping ();
    }
    catch (std::exception const &ex)
    {
      return false;
    }
  }
private:
  void kvs_ping ()
  {
    while (true)
    {
      if (! this->ping ())
      {
        ++m_ping_failed;

        if (m_ping_failed >= m_max_ping_failed)
        {
          MLOG (WARN, "lost connection to KVS, terminating...");
          _request_stop();
          return;
        }
      }
      else
      {
        m_ping_failed = 0;
      }

      boost::this_thread::sleep (m_ping_interval);
    }
  }

  boost::posix_time::time_duration m_ping_interval;
  unsigned int m_ping_failed;
  unsigned int m_max_ping_failed;

  //! \todo don't be pointer!
  fhg::com::kvs::client::kvsc* _kvs_client;

  boost::function<void()> _request_stop;

  //! \todo don't be pointer!
  boost::thread* _ping_thread;
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
