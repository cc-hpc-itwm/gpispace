#define BOOST_TEST_MODULE NetworkStrategyTest
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <fhglog/minimal.hpp>
#include <sdpa/com/NetworkStrategy.hpp>

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
  sdpa::com::NetworkStrategy net ( "discard"
                                 , "peer-1"
                                 , fhg::com::host_t ("localhost")
                                 , fhg::com::port_t ("0")
                                 );

  net.onStageStart ("dummy");
  net.onStageStop ("dummy");
}

BOOST_AUTO_TEST_SUITE_END()
