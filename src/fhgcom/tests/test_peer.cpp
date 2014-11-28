#define BOOST_TEST_MODULE PeerTest
#include <boost/test/unit_test.hpp>

#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/peer.hpp>
#include <fhgcom/peer_info.hpp>
#include <fhgcom/tcp_server.hpp>
#include <fhgcom/tests/address_printer.hpp>

#include <fhg/util/random_string.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>

struct KVSSetup
{
  KVSSetup ()
    : _io_service()
    , _io_service_work (_io_service)
    , m_kvsd()
    , m_serv (_io_service, m_kvsd, "localhost", "0", true)
    , _io_service_thread ([this] { _io_service.run(); })
    , _kvs ( new fhg::com::kvs::client::kvsc
             ( _kvs_client_io_service
             , "localhost"
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
    _io_service.stop();
    _io_service_thread.join();
  }

  boost::asio::io_service _io_service;
  boost::asio::io_service::work _io_service_work;
  fhg::com::kvs::server::kvsd m_kvsd;
  fhg::com::tcp_server m_serv;
  boost::thread _io_service_thread;
  boost::asio::io_service _kvs_client_io_service;
  fhg::com::kvs::kvsc_ptr_t _kvs;
};

BOOST_FIXTURE_TEST_CASE (check_setup, KVSSetup)
{
  // make sure that the kvs is reachable...
  std::string const key (fhg::util::random_string());
  std::string const value (fhg::util::random_string());

  _kvs->put (key, value);

  const fhg::com::kvs::values_type v (_kvs->get (key));
  BOOST_REQUIRE_EQUAL (v.size(), 1);
  BOOST_REQUIRE_EQUAL (v.begin()->first, key);
  BOOST_REQUIRE_EQUAL (v.begin()->second, value);
  _kvs->del (key);
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
  void fail_on_kvs_error (boost::system::error_code const&)
  {
    BOOST_ERROR ("could not contact KVS...");
  }
}

BOOST_FIXTURE_TEST_CASE (peer_run_single, KVSSetup)
{
  using namespace fhg::com;
  boost::asio::io_service peer_1_io_service;
  peer_t peer_1 ( peer_1_io_service
                , "peer-1"
                , host_t("localhost")
                , port_t("1235")
                , _kvs
                , &fail_on_kvs_error
                );
  boost::thread thrd_1 (&peer_t::run, &peer_1);

  peer_1.start();

  peer_1.stop();
  thrd_1.join ();
}

namespace
{
  fhg::com::host_t host (boost::asio::ip::tcp::endpoint const& ep)
  {
    return fhg::com::host_t (ep.address().to_string());
  }
  fhg::com::port_t port (boost::asio::ip::tcp::endpoint const& ep)
  {
    return fhg::com::port_t (std::to_string (ep.port()));
  }
}

BOOST_FIXTURE_TEST_CASE (peer_run_two, KVSSetup)
{
  using namespace fhg::com;

  boost::asio::io_service peer_1_io_service;
  peer_t peer_1 ( peer_1_io_service
                , "peer-1"
                , host_t("localhost")
                , port_t("0")
                , _kvs
                , &fail_on_kvs_error
                );
  boost::thread thrd_1 (&peer_t::run, &peer_1);

  boost::asio::io_service peer_2_io_service;
  peer_t peer_2 ( peer_2_io_service
                , "peer-2"
                , host_t("localhost")
                , port_t("0")
                , _kvs
                , &fail_on_kvs_error
                );
  boost::thread thrd_2 (&peer_t::run, &peer_2);

  peer_1.start();
  peer_2.start();

  peer_1.send ( peer_1.connect_to ( host (peer_2.local_endpoint())
                                  , port (peer_2.local_endpoint())
                                  )
              , "hello world!"
              );
    message_t m;
    peer_2.recv (&m);

  BOOST_CHECK_EQUAL (m.header.src, p2p::address_t ("peer-1"));
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

  boost::asio::io_service peer_1_io_service;
  peer_t peer_1 ( peer_1_io_service
                , "peer-1"
                , host_t("localhost")
                , port_t("0")
                , _kvs
                , &fail_on_kvs_error
                );
  boost::thread thrd_1 (&peer_t::run, &peer_1);

  boost::asio::io_service peer_2_io_service;
  peer_t peer_2 ( peer_1_io_service
                , "peer-2"
                , host_t("localhost")
                , port_t("0")
                , _kvs
                , &fail_on_kvs_error
                );
  boost::thread thrd_2 (&peer_t::run, &peer_2);

  peer_1.start();
  peer_2.start();

  peer_1.stop();
  peer_2.stop();

  thrd_1.join ();
  thrd_2.join ();
}

BOOST_FIXTURE_TEST_CASE (peer_loopback, KVSSetup)
{
  using namespace fhg::com;

  boost::asio::io_service peer_1_io_service;
  peer_t peer_1 ( peer_1_io_service
                , "peer-1"
                , host_t("localhost")
                , port_t("0")
                , _kvs
                , &fail_on_kvs_error
                );
  boost::thread thrd_1 (&peer_t::run, &peer_1);

  peer_1.start();

  p2p::address_t const addr ( peer_1.connect_to ( host (peer_1.local_endpoint())
                                                , port (peer_1.local_endpoint())
                                                )
                            );

    for (std::size_t i (0); i < 10000; ++i)
    {
      peer_1.send(addr, "hello world!");
    }

  peer_1.stop();
  thrd_1.join ();
}

BOOST_FIXTURE_TEST_CASE (send_to_nonexisting_peer, KVSSetup)
{
  using namespace fhg::com;

  boost::asio::io_service peer_1_io_service;
  peer_t peer_1 ( peer_1_io_service
                , "peer-1"
                , host_t("localhost")
                , port_t("0")
                , _kvs
                , &fail_on_kvs_error
                );
  boost::thread thrd_1 (&peer_t::run, &peer_1);

  peer_1.start();

  BOOST_CHECK_THROW ( peer_1.connect_to
                        (host_t ("unknown host"), port_t ("unknown service"))
                    , std::exception
                    );

  peer_1.stop();
  thrd_1.join ();
}

BOOST_FIXTURE_TEST_CASE (send_large_data, KVSSetup)
{
  using namespace fhg::com;

  boost::asio::io_service peer_1_io_service;
  peer_t peer_1 ( peer_1_io_service
                , "peer-1"
                , host_t("localhost")
                , port_t("0")
                , _kvs
                , &fail_on_kvs_error
                );
  boost::thread thrd_1 (&peer_t::run, &peer_1);

  peer_1.start();

  peer_1.send( peer_1.connect_to ( host (peer_1.local_endpoint())
                                 , port (peer_1.local_endpoint())
                                 )
             , std::string (2<<25, 'X')
             );
    message_t r;
    peer_1.recv(&r);

    BOOST_CHECK_EQUAL(2<<25, r.data.size());

  peer_1.stop();
  thrd_1.join ();
}

BOOST_FIXTURE_TEST_CASE (peers_with_fixed_ports, KVSSetup)
{
  using namespace fhg::com;

  boost::asio::io_service peer_1_io_service;
  peer_t peer_1 ( peer_1_io_service
                , "peer-1"
                , host_t("localhost")
                , port_t("0")
                , _kvs
                , &fail_on_kvs_error
                );
  boost::thread thrd_1 (&peer_t::run, &peer_1);

  boost::asio::io_service peer_2_io_service;
  peer_t peer_2 ( peer_1_io_service
                , "peer-2"
                , host_t("localhost")
                , port_t("0")
                , _kvs
                , &fail_on_kvs_error
                );
  boost::thread thrd_2 (&peer_t::run, &peer_2);

  peer_1.start();
  peer_2.start();

  peer_1.send( peer_1.connect_to ( host (peer_2.local_endpoint())
                                 , port (peer_2.local_endpoint())
                                 )
             , "hello world!"
             );

  peer_1.stop();
  thrd_1.join ();

  peer_2.stop();
  thrd_2.join ();
}

BOOST_FIXTURE_TEST_CASE (peers_with_fixed_ports_reuse, KVSSetup)
{
  using namespace fhg::com;

  boost::asio::io_service peer_1_io_service;
  peer_t peer_1 ( peer_1_io_service
                , "peer-1"
                , host_t("localhost")
                , port_t("0")
                , _kvs
                , &fail_on_kvs_error
                );
  boost::thread thrd_1 (&peer_t::run, &peer_1);

  boost::asio::io_service peer_2_io_service;
  peer_t peer_2 ( peer_1_io_service
                , "peer-2"
                , host_t("localhost")
                , port_t("0")
                , _kvs
                , &fail_on_kvs_error
                );
  boost::thread thrd_2 (&peer_t::run, &peer_2);

  peer_1.start();
  peer_2.start();

  peer_1.send ( peer_1.connect_to ( host (peer_2.local_endpoint())
                                  , port (peer_2.local_endpoint())
                                  )
              , "hello world!"
              );

  peer_1.stop();
  thrd_1.join ();

  peer_2.stop();
  thrd_2.join ();
}

BOOST_FIXTURE_TEST_CASE (two_peers_one_restarts_repeatedly, KVSSetup)
{
  using namespace fhg::com;

  boost::asio::io_service peer_1_io_service;
  peer_t peer_1 ( peer_1_io_service
                , "peer-1"
                , host_t("localhost")
                , port_t("0")
                , _kvs
                , &fail_on_kvs_error
                );
  boost::thread thrd_1 (&peer_t::run, &peer_1);

  peer_1.start();

  bool stop_request (false);

  //! \note Race: Possibly taken. Needs to be known before peer
  //! starts, though, to allow reconnecting.
  boost::asio::ip::tcp::endpoint peer_2_endpoint
    (boost::asio::ip::address::from_string ("127.0.0.1"), 15123);

  boost::thread sender ( [&peer_1, &stop_request, &peer_2_endpoint]
                       {
                         while (not stop_request)
                         {
                           try
                           {
                             peer_1.send ( peer_1.connect_to
                                             ( host (peer_2_endpoint)
                                             , port (peer_2_endpoint)
                                             )
                                         , "hello world\n"
                                         );
                           }
                           catch (std::exception const &ex)
                           {
                             //! \todo explain why this can be ignored
                             // ignore
                           }
                         }
                       }
                       );

  for (std::size_t i (0); i < 100; ++i)
  {
    boost::asio::io_service peer_2_io_service;
    peer_t peer_2 ( peer_2_io_service
                  , "peer-2"
                  , host_t (peer_2_endpoint.address().to_string())
                  , port_t (std::to_string (peer_2_endpoint.port()))
                  , _kvs
                  , &fail_on_kvs_error
                  );
    boost::thread thrd_2 (&peer_t::run, &peer_2);

    try
    {
      peer_2.start();
      peer_2.send ( peer_2.connect_to ( host (peer_1.local_endpoint())
                                      , port (peer_1.local_endpoint())
                                      )
                  , "hello world!"
                  );
    }
    catch (boost::system::system_error const &se)
    {
      using namespace boost::system;

      if (se.code ().value () != boost::asio::error::eof)
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

    peer_2.stop ();
    thrd_2.join ();
  }

  stop_request = true;

  sender.join ();

  peer_1.stop();
  thrd_1.join ();
}
