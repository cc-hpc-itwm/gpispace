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
    : m_pool (new fhg::com::io_service_pool(1))
    , m_kvsd (new fhg::com::kvs::server::kvsd)
    , m_serv ( new fhg::com::tcp_server
               (*m_pool, *m_kvsd, kvs_host (), kvs_port (), true)
             )
    , m_thrd
      (new boost::thread (boost::bind (&fhg::com::io_service_pool::run, m_pool)))
  {
    _kvs = fhg::com::kvs::kvsc_ptr_t
      ( new fhg::com::kvs::client::kvsc
        ( kvs_host()
        , boost::lexical_cast<std::string>(m_serv->port())
        , true // auto_reconnect
        , boost::posix_time::seconds (10)
        , 3
        )
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
  }

  fhg::com::kvs::kvsc_ptr_t _kvs;

  fhg::com::io_service_pool *m_pool;
  fhg::com::kvs::server::kvsd *m_kvsd;
  fhg::com::tcp_server *m_serv;
  boost::thread *m_thrd;
};

BOOST_FIXTURE_TEST_CASE (check_setup, KVSSetup)
{
  // make sure that the kvs is reachable...
  using namespace fhg::com;

  _kvs->put ("fhg.com.test.PeerTest", 42);

  const kvs::values_type v (_kvs->get ("fhg.com.test.PeerTest"));
  BOOST_REQUIRE_EQUAL (v.size(), 1);
  BOOST_REQUIRE_EQUAL (v.begin()->first, "fhg.com.test.PeerTest");
  BOOST_REQUIRE_EQUAL (v.begin()->second, "42");
  _kvs->del ("fhg.com.test.PeerTest");
}

BOOST_FIXTURE_TEST_CASE (output_test, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_o ("peer", host_t("localhost"), port_t("1235"), _kvs);

  BOOST_CHECK_EQUAL (peer_o.name(), "peer");
  BOOST_CHECK_EQUAL (peer_o.host(), "localhost");
  BOOST_CHECK_EQUAL (peer_o.port(), "1235");

  std::stringstream sstr;
  sstr << peer_o;
  BOOST_CHECK_EQUAL (sstr.str(), "peer@[localhost]:1235");
}

BOOST_AUTO_TEST_CASE (parse_peer_info_full)
{
  using namespace fhg::com;

  const std::string url ("peer@[::1]:1234");
  peer_info_t pi = peer_info_t::from_string (url);

  BOOST_CHECK_EQUAL (pi.name(), "peer");
  BOOST_CHECK_EQUAL (pi.host(), "::1");
  BOOST_CHECK_EQUAL (pi.port(), "1234");
}

BOOST_AUTO_TEST_CASE (parse_peer_info_wo_name)
{
  using namespace fhg::com;

  const std::string url ("[::1]:1234");
  peer_info_t pi = peer_info_t::from_string (url);

  BOOST_CHECK_EQUAL (pi.name(), "");
  BOOST_CHECK_EQUAL (pi.host(), "::1");
  BOOST_CHECK_EQUAL (pi.port(), "1234");
}

BOOST_AUTO_TEST_CASE (parse_peer_info_wo_port)
{
  using namespace fhg::com;

  const std::string url ("localhost");

  BOOST_REQUIRE_THROW ( peer_info_t::from_string (url)
                      , std::runtime_error
                      );
}

BOOST_AUTO_TEST_CASE (parse_peer_info_wi_name)
{
  using namespace fhg::com;

  const std::string url ("peer@localhost:1234");
  peer_info_t pi = peer_info_t::from_string (url);

  BOOST_CHECK_EQUAL (pi.name(), "peer");
  BOOST_CHECK_EQUAL (pi.host(), "localhost");
  BOOST_CHECK_EQUAL (pi.port(), "1234");
}

BOOST_FIXTURE_TEST_CASE (peer_run_single, KVSSetup)
{
  using namespace fhg::com;
  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("1235"), _kvs);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_1.start();

  peer_1.stop();
  thrd_1.join ();
}

BOOST_FIXTURE_TEST_CASE (peer_run_two, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("0"), _kvs);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2 ("peer-2", host_t("localhost"), port_t("0"), _kvs);
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

BOOST_FIXTURE_TEST_CASE (resolve_peer_names, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("0"), _kvs);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2 ("peer-2", host_t("localhost"), port_t("0"), _kvs);
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

BOOST_FIXTURE_TEST_CASE (resolve_peer_addresses, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("0"), _kvs);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2 ("peer-2", host_t("localhost"), port_t("0"), _kvs);
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

BOOST_FIXTURE_TEST_CASE (peer_loopback, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("0"), _kvs);
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

BOOST_FIXTURE_TEST_CASE (send_to_nonexisting_peer, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("0"), _kvs);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_1.start();

  BOOST_CHECK_THROW ( peer_1.send("some-unknown-peer", "hello world!")
                    , std::exception
                    );

  peer_1.stop();
  thrd_1.join ();
}

BOOST_FIXTURE_TEST_CASE (send_large_data, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("0"), _kvs);
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

BOOST_FIXTURE_TEST_CASE (peers_with_fixed_ports, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("15482"), _kvs);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2 ("peer-2", host_t("localhost"), port_t("15483"), _kvs);
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

BOOST_FIXTURE_TEST_CASE (peers_with_fixed_ports_reuse, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("15482"), _kvs);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2 ("peer-2", host_t("localhost"), port_t("15483"), _kvs);
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

static void s_send_loop ( fhg::com::peer_t *peer
                        , bool *stop
                        , std::string const & to
                        )
{
  while (not *stop)
  {
    try
    {
      usleep (500);
      peer->send (to, "hello world\n");
    }
    catch (std::exception const &ex)
    {
      // ignore
    }
  }
}

BOOST_FIXTURE_TEST_CASE (two_peers_one_restarts_repeatedly, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1 ("peer-1", host_t("localhost"), port_t("15482"), _kvs);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));
  peer_1.start();

  bool stop_request (false);

  boost::thread sender (boost::bind ( s_send_loop
                                    , &peer_1
                                    , &stop_request
                                    , "peer-2"
                                    )
                       );

  for (std::size_t i (0); i < 1000; ++i)
  {
    peer_t peer_2 ("peer-2", host_t("localhost"), port_t("15483"), _kvs);
    boost::thread thrd_2 (boost::bind (&peer_t::run, &peer_2));

    try
    {
      peer_2.start();
      peer_2.send (peer_1.name (), "hello world!");
      peer_2.stop();
    }
    catch (boost::system::system_error const &se)
    {
      using namespace boost::system;

      if (  se.code ().value () != errc::address_not_available
         )
      {
        peer_2.stop ();
        BOOST_ERROR (se.what ());
      }
    }
    catch (std::exception const & ex)
    {
      peer_2.stop ();
      BOOST_ERROR ( ex.what() );
    }

    thrd_2.join ();
  }

  stop_request = true;

  sender.join ();

  peer_1.stop();
  thrd_1.join ();
}
