#include "kvs.hpp"

#include <fhglog/LogMacros.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/util/read_bool.hpp>

#include <fhgcom/kvs/kvsc.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

class kvs_client
{
public:
  kvs_client ( std::string host
             , std::string port
             , boost::posix_time::time_duration ping_interval
             , unsigned int max_ping_failed
             , boost::function<void()> request_stop
             , boost::posix_time::time_duration timeout
             )
    : _ping_interval (ping_interval)
    , _counter_ping_failed (0)
    , _max_ping_failed (max_ping_failed)
    , _request_stop (request_stop)
    , _kvs_client
      ( !host.empty() ? host : throw std::runtime_error ("kvs host empty")
      , !port.empty() ? port : throw std::runtime_error ("kvs port empty")
      , true // auto_reconnect
      , timeout
      , 1 // max_connection_attempts
      )
    , _ping_thread (&kvs_client::check_for_ping, this)
  {}

  ~kvs_client()
  {
    _ping_thread.interrupt();
    if (_ping_thread.joinable())
    {
      _ping_thread.join();
    }
  }

  std::string get (std::string const & k, std::string const &dflt) const
  {
    try
    {
      std::map<std::string, std::string>  v (list (k));
      if (v.size() == 1)
      {
        return boost::lexical_cast<std::string>(v.begin()->second);
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

  void put (std::string const & k, std::string const &value)
  {
    _kvs_client.put (k, value);
  }

  void del (std::string const & k)
  {
    _kvs_client.del (k);
  }

  int inc (std::string const & k, int step)
  {
    return _kvs_client.inc (k, step);
  }

  std::map<std::string, std::string> list (std::string const &prefix) const
  {
    return _kvs_client.get (prefix);
  }

private:
  void check_for_ping()
  {
    while (true)
    {
      bool ping_failed (false);
      try
      {
        ping_failed = _kvs_client.ping ();
      }
      catch (std::exception const &ex)
      {
        ping_failed = true;
      }

      if (ping_failed)
      {
        ++_counter_ping_failed;

        if (_counter_ping_failed >= _max_ping_failed)
        {
          MLOG (WARN, "lost connection to KVS, terminating...");
          _request_stop();
          return;
        }
      }
      else
      {
        _counter_ping_failed = 0;
      }

      boost::this_thread::sleep (_ping_interval);
    }
  }

  boost::posix_time::time_duration _ping_interval;
  unsigned int _counter_ping_failed;
  unsigned int _max_ping_failed;

  boost::function<void()> _request_stop;

  fhg::com::kvs::client::kvsc _kvs_client;

  boost::thread _ping_thread;
};

class KeyValueStorePlugin : FHG_PLUGIN
                          , public kvs::KeyValueStore
{
public:
  FHG_PLUGIN_START()
  try
  {
    const std::string host (fhg_kernel()->get ("host", "localhost"));
    const std::string port (fhg_kernel()->get ("port", "2439"));
    const unsigned int max_ping_failed (fhg_kernel ()->get ("max_ping_failed", 3));
    const boost::posix_time::time_duration ping_interval
      ( boost::posix_time::duration_from_string
        ( fhg_kernel ()->get<std::string>
          ( "ping"
          , boost::posix_time::to_simple_string (boost::posix_time::seconds (5))
          )
        )
      );
    const boost::posix_time::time_duration timeout
      ( boost::posix_time::duration_from_string
        ( fhg_kernel ()->get<std::string>
          ( "timeout"
          , boost::posix_time::to_simple_string (boost::posix_time::seconds (120))
          )
        )
      );

    _kvs_client_impl = new kvs_client
      ( host
      , port
      , ping_interval
      , max_ping_failed
      , boost::bind (&fhg::plugin::Kernel::shutdown, fhg_kernel())
      , timeout
      );

    fhg::com::kvs::get_or_create_global_kvs (host, port, true, timeout, 1);

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
    delete _kvs_client_impl;
    _kvs_client_impl = NULL;

    FHG_PLUGIN_STOPPED();
  }

  value_type get (key_type const & k, value_type const &dflt) const
  {
    return _kvs_client_impl->get (k, dflt);
  }

  void put (key_type const & k, value_type const &value)
  {
    _kvs_client_impl->put (k, value);
  }

  void del (key_type const & k)
  {
    _kvs_client_impl->del (k);
  }

  int inc (key_type const & k, int step)
  {
    return _kvs_client_impl->inc (k, step);
  }

  key_value_map_type list() const
  {
    return _kvs_client_impl->list ("");
  }

  key_value_map_type list (key_type const &prefix) const
  {
    return _kvs_client_impl->list (prefix);
  }

private:
  //! \todo don't be pointer!
  kvs_client* _kvs_client_impl;
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
