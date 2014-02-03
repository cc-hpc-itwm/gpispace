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

    fhg::com::kvs::get_or_create_global_kvs
      ( !host.empty() ? host : throw std::runtime_error ("kvs host empty")
      , !port.empty() ? port : throw std::runtime_error ("kvs port empty")
      , true // auto_reconnect
      , timeout
      , 1 // max_connection_attempts
      );

    _kvs_client_impl = new fhg::com::kvs::pinging_kvs_client
      ( ping_interval
      , max_ping_failed
      , boost::bind (&fhg::plugin::Kernel::shutdown, fhg_kernel())
      , fhg::com::kvs::global_kvs()
      );

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

    delete *fhg::com::kvs::global::get_kvs_info_ptr();
    *fhg::com::kvs::global::get_kvs_info_ptr() = NULL;

    FHG_PLUGIN_STOPPED();
  }

  value_type get (key_type const & k, value_type const &dflt) const
  {
    std::map<std::string, std::string>  v (_kvs_client_impl->list (k));
    if (v.size() == 1)
    {
      return v.begin()->second;
    }
    else
    {
      //! \todo Should be throw: obviously bogus data.
      return dflt;
    }
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
  fhg::com::kvs::pinging_kvs_client* _kvs_client_impl;
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
