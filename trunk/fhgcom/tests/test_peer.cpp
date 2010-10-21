#define BOOST_TEST_MODULE PeerTest
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fhgcom/peer.hpp>

BOOST_AUTO_TEST_CASE ( start_stop_test )
{
  using namespace fhg::com;

  peer_t peer ("peer", host_t("localhost"), port_t("1234"));
  peer.start();
  peer.stop();
}

BOOST_AUTO_TEST_CASE ( output_test )
{
  using namespace fhg::com;

  peer_t peer_o ("peer", host_t("localhost"), port_t("1234"));

  BOOST_CHECK_EQUAL (peer_o.name(), "peer");
  BOOST_CHECK_EQUAL (peer_o.host(), "localhost");
  BOOST_CHECK_EQUAL (peer_o.port(), "1234");

  std::stringstream sstr;
  sstr << peer_o;
  BOOST_CHECK_EQUAL (sstr.str(), "peer@[localhost]:1234");
}
