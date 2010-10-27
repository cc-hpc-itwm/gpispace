#define BOOST_TEST_MODULE PeerTest
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <fhgcom/peer.hpp>
#include <fhgcom/peer_io.hpp>
#include <fhgcom/peer_info.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>

// TODO: move to fixture
BOOST_AUTO_TEST_CASE ( setup_dummy )
{
  FHGLOG_SETUP();

  // make sure that the kvs is reachable...
  using namespace fhg::com;

  kvs::put ("fhg.com.test.PeerTest", 42);
  int i = kvs::get<int>("fhg.com.test.PeerTest");
  kvs::del ("fhg.com.test.PeerTest");

  BOOST_CHECK_EQUAL (42, i);
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

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("0"));
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2 ("peer-2", host_t("localhost"), port_t("0"));
  boost::thread thrd_2 (boost::bind (&peer_t::run, &peer_2));

  peer_1.start();
  peer_2.start();

  try
  {
    peer_1.send ("peer-2", "hello world!");
    message_t m;
    peer_2.recv (&m);

    std::string src;
    peer_2.resolve_addr(m.header.src, src);
    std::string data (m.data.begin(), m.data.end());
    BOOST_CHECK_EQUAL (src, "peer-1");
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

BOOST_AUTO_TEST_CASE ( resolve_peer_names )
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("0"));
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2 ("peer-2", host_t("localhost"), port_t("0"));
  boost::thread thrd_2 (boost::bind (&peer_t::run, &peer_2));

  peer_1.start();
  peer_2.start();

  {
    p2p::address_t a1;
    peer_1.resolve_name ("peer-1", a1);
    p2p::address_t a2;
    peer_2.resolve_name ("peer-1", a2);
    BOOST_CHECK_EQUAL (a1, a2);
  }

  {
    p2p::address_t a1;
    peer_1.resolve_name ("peer-2", a1);
    p2p::address_t a2;
    peer_2.resolve_name ("peer-2", a2);
    BOOST_CHECK_EQUAL (a1, a2);
  }

  peer_1.stop();
  peer_2.stop();

  thrd_1.join ();
  thrd_2.join ();
}

BOOST_AUTO_TEST_CASE ( resolve_peer_addresses )
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("0"));
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2 ("peer-2", host_t("localhost"), port_t("0"));
  boost::thread thrd_2 (boost::bind (&peer_t::run, &peer_2));

  peer_1.start();
  peer_2.start();

  const p2p::address_t peer_1_addr (peer_1.address());
  const p2p::address_t peer_2_addr (peer_2.address());

  {
    std::string n_1;
    peer_1.resolve_addr (peer_1_addr, n_1);

    std::string n_2;
    peer_2.resolve_addr (peer_1_addr, n_2);

    BOOST_CHECK_EQUAL (n_1, n_2);
  }
  {
    std::string n_1;
    peer_1.resolve_addr (peer_2_addr, n_1);

    std::string n_2;
    peer_2.resolve_addr (peer_2_addr, n_2);

    BOOST_CHECK_EQUAL (n_1, n_2);
  }

  peer_1.stop();
  peer_2.stop();

  thrd_1.join ();
  thrd_2.join ();
}

BOOST_AUTO_TEST_CASE ( peer_loopback )
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("0"));
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_1.start();

  try
  {
    for (std::size_t i (0); i < 10000; ++i)
    {
      peer_1.send(peer_1.name (), "hello world!");
    }
  }
  catch (std::exception const & ex)
  {
    BOOST_ERROR ( ex.what() );
  }

  peer_1.stop();
  thrd_1.join ();
}

BOOST_AUTO_TEST_CASE ( send_to_nonexisting_peer )
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("0"));
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_1.start();

  try
  {
    peer_1.send("peer-2", "hello world!");
  }
  catch (std::exception const & ex)
  {
    BOOST_ERROR ( ex.what() );
  }

  peer_1.stop();
  thrd_1.join ();
}

BOOST_AUTO_TEST_CASE ( send_large_data )
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("0"));
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_1.start();

  try
  {
    message_t m;
    peer_1.resolve_name("peer-1", m.header.dst);
    m.data.resize( (2<<25) );
    m.header.length = m.data.size();
    peer_1.send(&m);
    message_t r;
    peer_1.recv(&r);

    BOOST_CHECK_EQUAL(m.data.size(), r.data.size());
  }
  catch (std::exception const & ex)
  {
    BOOST_ERROR ( ex.what() );
  }

  peer_1.stop();
  thrd_1.join ();
}
