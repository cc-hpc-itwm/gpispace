#define BOOST_TEST_MODULE PeerTest
#include <boost/test/unit_test.hpp>

#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/peer.hpp>
#include <fhgcom/peer_info.hpp>
#include <fhgcom/tcp_server.hpp>
#include <fhgcom/tests/address_printer.hpp>

#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <iostream>

struct KVSSetup
{
  KVSSetup ()
    : m_pool (1)
    , m_kvsd (boost::none)
    , m_serv (m_pool, m_kvsd, "localhost", "0", true)
    , m_thrd (&fhg::com::io_service_pool::run, &m_pool)
    , _kvs ( new fhg::com::kvs::client::kvsc
             ( "localhost"
             , boost::lexical_cast<std::string> (m_serv.port())
             , true // auto_reconnect
             , boost::posix_time::seconds (10)
             , 3
             )
           )
  {}

  ~KVSSetup ()
  {
    m_serv.stop ();
    m_pool.stop ();
    m_thrd.join ();
  }

  fhg::com::io_service_pool m_pool;
  fhg::com::kvs::server::kvsd m_kvsd;
  fhg::com::tcp_server m_serv;
  boost::thread m_thrd;
  fhg::com::kvs::kvsc_ptr_t _kvs;
};

BOOST_FIXTURE_TEST_CASE (check_setup, KVSSetup)
{
  // make sure that the kvs is reachable...
  _kvs->put ("fhg.com.test.PeerTest", 42);

  const fhg::com::kvs::values_type v (_kvs->get ("fhg.com.test.PeerTest"));
  BOOST_REQUIRE_EQUAL (v.size(), 1);
  BOOST_REQUIRE_EQUAL (v.begin()->first, "fhg.com.test.PeerTest");
  BOOST_REQUIRE_EQUAL (v.begin()->second, "42");
  _kvs->del ("fhg.com.test.PeerTest");
}

BOOST_AUTO_TEST_CASE (parse_peer_info_full)
{
  fhg::com::peer_info_t ("peer@[::1]:1234");
}

BOOST_AUTO_TEST_CASE (parse_peer_info_wo_name)
{
  fhg::com::peer_info_t ("[::1]:1234");
}

namespace
{
  void ctor (std::string const& url)
  {
    (void)fhg::com::peer_info_t (url);
  }
}

BOOST_AUTO_TEST_CASE (parse_peer_info_wo_port)
{
  BOOST_REQUIRE_THROW ( ctor ("localhost")
                      , std::runtime_error
                      );
}

BOOST_AUTO_TEST_CASE (parse_peer_info_wi_name)
{
  fhg::com::peer_info_t ("peer@localhost:1234");
}

namespace
{
  void ignore_errors (boost::system::error_code const&)
  {}
}

BOOST_FIXTURE_TEST_CASE (peer_run_single, KVSSetup)
{
  using namespace fhg::com;
  peer_t peer_1
    ("peer-1", host_t("localhost"), port_t("1235"), _kvs, &ignore_errors);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_1.start();

  peer_1.stop();
  thrd_1.join ();
}

BOOST_FIXTURE_TEST_CASE (peer_run_two, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1
    ("peer-1", host_t("localhost"), port_t("0"), _kvs, &ignore_errors);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2
    ("peer-2", host_t("localhost"), port_t("0"), _kvs, &ignore_errors);
  boost::thread thrd_2 (boost::bind (&peer_t::run, &peer_2));

  peer_1.start();
  peer_2.start();

    peer_1.send ("peer-2", "hello world!");
    message_t m;
    peer_2.recv (&m);

  BOOST_CHECK_EQUAL (peer_2.resolve_addr (m.header.src), "peer-1");
  BOOST_CHECK_EQUAL
    (std::string (m.data.begin(), m.data.end()), "hello world!");

  peer_1.stop();
  peer_2.stop();

  thrd_1.join ();
  thrd_2.join ();
}

BOOST_FIXTURE_TEST_CASE (resolve_peer_names, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1
    ("peer-1", host_t("localhost"), port_t("0"), _kvs, &ignore_errors);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2
    ("peer-2", host_t("localhost"), port_t("0"), _kvs, &ignore_errors);
  boost::thread thrd_2 (boost::bind (&peer_t::run, &peer_2));

  peer_1.start();
  peer_2.start();

  peer_1.stop();
  peer_2.stop();

  thrd_1.join ();
  thrd_2.join ();
}

BOOST_FIXTURE_TEST_CASE (resolve_peer_addresses, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1
    ("peer-1", host_t("localhost"), port_t("0"), _kvs, &ignore_errors);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2
    ("peer-2", host_t("localhost"), port_t("0"), _kvs, &ignore_errors);
  boost::thread thrd_2 (boost::bind (&peer_t::run, &peer_2));

  peer_1.start();
  peer_2.start();

  BOOST_CHECK_EQUAL ( peer_1.resolve_addr (peer_1.address())
                    , peer_2.resolve_addr (peer_1.address())
                    );

  BOOST_CHECK_EQUAL ( peer_1.resolve_addr (peer_2.address())
                    , peer_2.resolve_addr (peer_2.address())
                    );

  peer_1.stop();
  peer_2.stop();

  thrd_1.join ();
  thrd_2.join ();
}

BOOST_FIXTURE_TEST_CASE (peer_loopback, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1
    ("peer-1", host_t("localhost"), port_t("0"), _kvs, &ignore_errors);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_1.start();

    for (std::size_t i (0); i < 10000; ++i)
    {
      peer_1.send(peer_1.name (), "hello world!");
    }

  peer_1.stop();
  thrd_1.join ();
}

BOOST_FIXTURE_TEST_CASE (send_to_nonexisting_peer, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1
    ("peer-1", host_t("localhost"), port_t("0"), _kvs, &ignore_errors);
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

  peer_t peer_1
    ("peer-1", host_t("localhost"), port_t("0"), _kvs, &ignore_errors);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_1.start();

    message_t m;
    m.header.dst = fhg::com::p2p::address_t ("peer-1");
    m.data.resize( (2<<25) );
    m.header.length = m.data.size();
    peer_1.send(&m);
    message_t r;
    peer_1.recv(&r);

    BOOST_CHECK_EQUAL(m.data.size(), r.data.size());

  peer_1.stop();
  thrd_1.join ();
}

BOOST_FIXTURE_TEST_CASE (peers_with_fixed_ports, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1
    ("peer-1", host_t("localhost"), port_t("15482"), _kvs, &ignore_errors);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2
    ("peer-2", host_t("localhost"), port_t("15483"), _kvs, &ignore_errors);
  boost::thread thrd_2 (boost::bind (&peer_t::run, &peer_2));

  peer_1.start();
  peer_2.start();

    peer_1.send(peer_2.name (), "hello world!");

  peer_1.stop();
  thrd_1.join ();

  peer_2.stop();
  thrd_2.join ();
}

BOOST_FIXTURE_TEST_CASE (peers_with_fixed_ports_reuse, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1
    ("peer-1", host_t("localhost"), port_t("15482"), _kvs, &ignore_errors);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));

  peer_t peer_2
    ("peer-2", host_t("localhost"), port_t("15483"), _kvs, &ignore_errors);
  boost::thread thrd_2 (boost::bind (&peer_t::run, &peer_2));

  peer_1.start();
  peer_2.start();

    peer_1.send(peer_2.name (), "hello world!");

  peer_1.stop();
  thrd_1.join ();

  peer_2.stop();
  thrd_2.join ();
}

namespace
{
  void send_loop ( fhg::com::peer_t *peer
                 , bool *stop
                 , std::string const & to
                 )
  {
    while (not *stop)
    {
      try
      {
        peer->send (to, "hello world\n");
      }
      catch (std::exception const &ex)
      {
        // ignore
      }
    }
  }
}

BOOST_FIXTURE_TEST_CASE (two_peers_one_restarts_repeatedly, KVSSetup)
{
  using namespace fhg::com;

  peer_t peer_1
    ("peer-1", host_t("localhost"), port_t("15482"), _kvs, &ignore_errors);
  boost::thread thrd_1 (boost::bind (&peer_t::run, &peer_1));
  peer_1.start();

  bool stop_request (false);

  boost::thread sender (boost::bind ( send_loop
                                    , &peer_1
                                    , &stop_request
                                    , "peer-2"
                                    )
                       );

  for (std::size_t i (0); i < 100; ++i)
  {
    peer_t peer_2
      ("peer-2", host_t("localhost"), port_t("15483"), _kvs, &ignore_errors);
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
