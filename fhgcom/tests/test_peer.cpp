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

static const std::string &kvs_host () { static std::string s("localhost"); return s; }
static const std::string &kvs_port () { static std::string s("0"); return s; }

struct KVSSetup
{
  KVSSetup ()
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

    LOG(INFO, "kvs daemon is listening on port " << m_serv->port ());

    fhg::com::kvs::global::get_kvs_info().init( kvs_host()
                                              , boost::lexical_cast<std::string>(m_serv->port())
                                              , boost::posix_time::seconds(10)
                                              , 3
                                              );
  }

  ~KVSSetup ()
  {
    m_serv->stop ();
    m_pool->stop ();
    m_thrd->join ();
    delete m_thrd;
    delete m_serv;
    delete m_kvsd;
    delete m_pool;

    delete *fhg::com::kvs::global::get_kvs_info_ptr ();
    *fhg::com::kvs::global::get_kvs_info_ptr () = 0;

    FHGLOG_TERM ();
  }

  fhg::com::io_service_pool *m_pool;
  fhg::com::kvs::server::kvsd *m_kvsd;
  fhg::com::tcp_server *m_serv;
  boost::thread *m_thrd;
};

BOOST_GLOBAL_FIXTURE (KVSSetup);

struct F
{
};

BOOST_FIXTURE_TEST_SUITE( s, F )

BOOST_AUTO_TEST_CASE ( check_setup )
{
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

  peer_t peer_o ("peer", host_t("localhost"), port_t("1235"));

  BOOST_CHECK_EQUAL (peer_o.name(), "peer");
  BOOST_CHECK_EQUAL (peer_o.host(), "localhost");
  BOOST_CHECK_EQUAL (peer_o.port(), "1235");

  std::stringstream sstr;
  sstr << peer_o;
  BOOST_CHECK_EQUAL (sstr.str(), "peer@[localhost]:1235");
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
  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("1235"));
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
    peer_1.send("some-unknown-peer", "hello world!");
    BOOST_ERROR ( "send to unknown peer did not fail with an exception!" );
  }
  catch (std::exception const & ex)
  {
    // ok
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

BOOST_AUTO_TEST_CASE ( peers_with_fixed_ports )
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("15482"));
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2 ("peer-2", host_t("localhost"), port_t("15483"));
  boost::thread thrd_2 (boost::bind (&peer_t::run, &peer_2));

  peer_1.start();
  peer_2.start();

  try
  {
    peer_1.send(peer_2.name (), "hello world!");
  }
  catch (std::exception const & ex)
  {
    BOOST_ERROR ( ex.what() );
  }

  peer_1.stop();
  thrd_1.join ();

  peer_2.stop();
  thrd_2.join ();
}

BOOST_AUTO_TEST_CASE ( peers_with_fixed_ports_reuse )
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("15482"));
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2 ("peer-2", host_t("localhost"), port_t("15483"));
  boost::thread thrd_2 (boost::bind (&peer_t::run, &peer_2));

  peer_1.start();
  peer_2.start();

  try
  {
    peer_1.send(peer_2.name (), "hello world!");
  }
  catch (std::exception const & ex)
  {
    BOOST_ERROR ( ex.what() );
  }

  peer_1.stop();
  thrd_1.join ();

  peer_2.stop();
  thrd_2.join ();
}

static void send_loop (bool *stop_request, fhg::com::peer_t *peer)
{
  while (! *stop_request)
  {
    try
    {
      peer->send ("peer-2", "hello world");
      usleep (500);
    }
    catch (boost::thread_interrupted const &)
    {
      break;
    }
    catch (std::exception const &ex)
    {
      // ignore
    }
  }
}

BOOST_AUTO_TEST_CASE ( two_peers_one_restarts_repeatedly )
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("15482"));
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));
  peer_1.start();

  bool stop_request (false);
  boost::thread thrd_send_recv (boost::bind (send_loop, &stop_request, &peer_1));

  for (std::size_t i (0); i < 1000; ++i)
  {
    peer_t peer_2 ("peer-2", host_t("localhost"), port_t("15483"));
    boost::thread thrd_2 (boost::bind (&peer_t::run, &peer_2));
    peer_2.start();

    try
    {
      peer_2.send(peer_1.name (), "hello world!");
    }
    catch (std::exception const & ex)
    {
      BOOST_ERROR ( ex.what() );
    }

    peer_2.stop();
    thrd_2.join ();
  }

  stop_request = true;
  thrd_send_recv.interrupt ();
  thrd_send_recv.join ();

  peer_1.stop();
  thrd_1.join ();
}

BOOST_AUTO_TEST_SUITE_END()
