#define BOOST_TEST_MODULE NetworkStrategyTest
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <fhglog/minimal.hpp>
#include <sdpa/com/NetworkStrategy.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <seda/Stage.hpp>
#include <seda/StageFactory.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/DiscardStrategy.hpp>
#include <seda/EventCountStrategy.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>

static const std::string &kvs_host () { static std::string s("localhost"); return s; }
static const std::string &kvs_port () { static std::string s("1234"); return s; }

struct F
{
  F ()
    : m_pool (0)
    , m_kvsd (0)
    , m_serv (0)
    , m_thrd (0)
  {
    FHGLOG_SETUP();

    m_pool = new fhg::com::io_service_pool(1);
    m_kvsd = new fhg::com::kvs::server::kvsd;
    m_serv = new fhg::com::tcp_server ( *m_pool
                                      , *m_kvsd
                                      , kvs_host ()
                                      , kvs_port ()
                                      , true
                                      );
    m_thrd = new boost::thread (boost::bind ( &fhg::com::io_service_pool::run
                                            , m_pool
                                            )
                               );

    m_serv->start();

    fhg::com::kvs::get_or_create_global_kvs ( kvs_host()
                                            , kvs_port()
                                            , true
                                            , boost::posix_time::seconds(3)
                                            , 1
                                            );
  }

  ~F ()
  {
    m_serv->stop ();
    m_pool->stop ();
    m_thrd->join ();
    delete m_thrd;
    delete m_serv;
    delete m_kvsd;
    delete m_pool;
  }

  fhg::com::io_service_pool *m_pool;
  fhg::com::kvs::server::kvsd *m_kvsd;
  fhg::com::tcp_server *m_serv;
  boost::thread *m_thrd;
};

BOOST_FIXTURE_TEST_SUITE( s, F )

BOOST_AUTO_TEST_CASE ( perform_test )
{
  seda::StageFactory::Ptr sFactory(new seda::StageFactory());

  seda::EventCountStrategy *ecs (0);
  seda::Strategy::Ptr discard (new seda::DiscardStrategy());
  ecs = new seda::EventCountStrategy(discard);
  discard = seda::Strategy::Ptr(ecs);
  seda::Stage::Ptr final (sFactory->createStage("count", discard));

  sdpa::com::NetworkStrategy::Ptr net
    (new sdpa::com::NetworkStrategy( "count"
                                   , "peer-1"
                                   , fhg::com::host_t ("localhost")
                                   , fhg::com::port_t ("0")
                                   )
    );
  seda::Stage::Ptr net_stage (sFactory->createStage ("net", net));

  seda::StageRegistry::instance().startAll();

  net->perform (seda::IEvent::Ptr(new sdpa::events::ErrorEvent( "peer-1"
                                                              , "peer-1"
                                                              , sdpa::events::ErrorEvent::SDPA_ENOERROR
                                                              , "success"
                                                              )
                                 )
               );

  ecs->wait (1, 1000);

  BOOST_CHECK_EQUAL (1u, ecs->count());

  seda::StageRegistry::instance().stopAll();
}

BOOST_AUTO_TEST_SUITE_END()
