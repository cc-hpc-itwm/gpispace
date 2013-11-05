#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <gspc/net.hpp>
#include <gspc/kvs/impl/kvs_net_service.hpp>

class KVSDImpl : FHG_PLUGIN
{
public:
  KVSDImpl ()
    : m_kvs ()
  {}

  ~KVSDImpl ()
  {}

  FHG_PLUGIN_START ()
  {
    m_kvs.reset
      (new gspc::kvs::service_t (fhg_kernel ()->get ("next", "inproc://")));

    gspc::net::handle ( "/service/kvs"
                      , gspc::net::service::strip_prefix ( "/service/kvs/"
                                                         , boost::ref (*m_kvs)
                                                         )
                      );

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP ()
  {
    gspc::net::unhandle ("/service/kvs");
    FHG_PLUGIN_STOPPED();
  }

private:
  boost::shared_ptr<gspc::kvs::service_t> m_kvs;
};

EXPORT_FHG_PLUGIN( kvsd
                 , KVSDImpl
                 , "kvsd"
                 , "provides the KVS service"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
