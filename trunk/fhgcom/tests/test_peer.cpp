#define BOOST_TEST_MODULE PeerTest
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <fhgcom/peer.hpp>
#include <fhgcom/peer_info.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>

BOOST_AUTO_TEST_CASE ( setup_dummy )
{
  FHGLOG_SETUP();
}

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

BOOST_AUTO_TEST_CASE ( parse_peer_info_full )
{
  using namespace fhg::com;

  const std::string url ("peer@[::1]:1234");
  peer_info_t pi = peer_info_t::from_string (url);

  BOOST_CHECK_EQUAL (pi.name(), "peer");
  BOOST_CHECK_EQUAL (pi.host(), "::1");
  BOOST_CHECK_EQUAL (pi.port(), "1234");
}

BOOST_AUTO_TEST_CASE ( parse_peer_info_wo_name )
{
  using namespace fhg::com;

  const std::string url ("[::1]:1234");
  peer_info_t pi = peer_info_t::from_string (url);

  BOOST_CHECK_EQUAL (pi.name(), "");
  BOOST_CHECK_EQUAL (pi.host(), "::1");
  BOOST_CHECK_EQUAL (pi.port(), "1234");
}

BOOST_AUTO_TEST_CASE ( parse_peer_info_wo_port )
{
  using namespace fhg::com;

  const std::string url ("localhost");
  try
  {
    peer_info_t pi = peer_info_t::from_string (url);
    BOOST_ERROR ("peer_info instantiation without port specification should fail");
  }
  catch (...)
  {
    // ok
  }
}

BOOST_AUTO_TEST_CASE ( parse_peer_info_wi_name )
{
  using namespace fhg::com;

  const std::string url ("peer@localhost:1234");
  peer_info_t pi = peer_info_t::from_string (url);

  BOOST_CHECK_EQUAL (pi.name(), "peer");
  BOOST_CHECK_EQUAL (pi.host(), "localhost");
  BOOST_CHECK_EQUAL (pi.port(), "1234");
}

BOOST_AUTO_TEST_CASE ( peer_run_single )
{
  using namespace fhg::com;
  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("1234"));
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_1.start();

  peer_1.stop();
  thrd_1.join ();
}

BOOST_AUTO_TEST_CASE ( peer_run_two )
{
  using namespace fhg::com;

/* // TODO: move to fixture
  const std::string kvs_host ("localhost");
  const std::string kvs_port ("6666");

  io_service_pool kvs_pool (1);
  kvs::server::kvsd kvsd ("");
  tcp_server kvs_server ( kvs_pool
                        , kvsd
                        , kvs_host
                        , kvs_port
                        , true
                        );

  kvs_server.start ();
  boost::thread kvs_thrd (boost::bind (&io_service_pool::run, &kvs_pool));
  kvs::get_or_create_global_kvs (kvs_host, kvs_port, true, boost::posix_time::seconds(10), 3);
*/

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("0"));
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2 ("peer-2", host_t("localhost"), port_t("0"));
  boost::thread thrd_2 (boost::bind (&peer_t::run, &peer_2));

  peer_1.start();
  peer_2.start();

  sleep (5);

  try
  {
    //    peer_1.set_location ("peer-2", host_t("localhost"), port_t("1234"));
    std::string from; std::string data;
    peer_2.async_recv (from, data);
    peer_1.async_send ("peer-2", "hello world!");

    sleep (1);

    BOOST_CHECK_EQUAL (from, "peer-1");
    BOOST_CHECK_EQUAL (data, "hello world!");
  }
  catch (std::exception const & ex)
  {
    BOOST_ERROR ( ex.what() );
  }


  peer_1.stop();
  peer_2.stop();

  thrd_1.join ();
  thrd_2.join ();
}
