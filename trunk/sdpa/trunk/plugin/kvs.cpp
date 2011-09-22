#include "kvs.hpp"

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhgcom/kvs/kvsc.hpp>

class KeyValueStorePlugin : FHG_PLUGIN
                          , public kvs::KeyValueStore
{
public:
  FHG_PLUGIN_START()
  {
    try
    {
      const std::string host(fhg_kernel()->get("host", "localhost"));
      const std::string port(fhg_kernel()->get("port", "2439"));

      LOG(INFO, "initializing KeyValueStore @ ["<<host<<"]:"<<port);

      fhg::com::kvs::global::get_kvs_info().init( host
                                                , port
                                                , boost::posix_time::seconds(2)
                                                , 1
                                                );
      fhg::com::kvs::global::get_kvs_info().start();
    }
    catch (std::exception const & ex)
    {
      LOG(ERROR, "could not connect to KVS: " << ex.what());
      LOG(INFO, "HINT: try setting plugin.kvs.host and/or plugin.kvs.port");
      FHG_PLUGIN_FAILED(ECONNREFUSED);
    }

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
};


EXPORT_FHG_PLUGIN( kvs
                 , KeyValueStorePlugin
                 , "provides access to a key value store"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
