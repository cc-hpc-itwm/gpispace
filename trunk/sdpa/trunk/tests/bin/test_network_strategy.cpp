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

struct F
{
  F()
  {
    FHGLOG_SETUP();
  }

  ~F()
  {
    BOOST_TEST_MESSAGE( "teardown fixture" );
  }
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

  seda::StageRegistry::instance().startAll();

  sdpa::com::NetworkStrategy net ( "count"
                                 , "peer-1"
                                 , fhg::com::host_t ("localhost")
                                 , fhg::com::port_t ("0")
                                 );

  net.onStageStart ("dummy");

  net.perform (seda::IEvent::Ptr(new sdpa::events::ErrorEvent( "peer-1"
                                                             , "peer-1"
                                                             , sdpa::events::ErrorEvent::SDPA_ENOERROR
                                                             , "success"
                                                             )
                                )
              );

  ecs->wait (1, 1000);

  BOOST_CHECK_EQUAL (1u, ecs->count());

  seda::StageRegistry::instance().stopAll();

  net.onStageStop ("dummy");
}

BOOST_AUTO_TEST_SUITE_END()
