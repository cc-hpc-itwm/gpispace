#include <boost/test/unit_test.hpp>

#include <fhgcom/peer.hpp>
#include <fhgcom/peer_info.hpp>
#include <fhgcom/tests/address_printer.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random/string.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/asio/io_service.hpp>

#include <thread>

namespace
{
  fhg::com::certificates_t const test_certificates
    (boost::filesystem::current_path()/"certs");
}

BOOST_TEST_DECORATOR (*boost::unit_test::timeout (2))
void test_peer_does_not_hang_when_resolve_throws
  (fhg::com::certificates_t const& certificates)
{
  fhg::util::testing::require_exception
    ( [&certificates]
      {
        fhg::com::peer_t ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                         , fhg::com::host_t ("NONONONONONONONONONO")
                         , fhg::com::port_t ("NONONONONONONONONONO")
                         , certificates
                         );
      }
    , boost::system::system_error
        (boost::asio::error::service_not_found, "resolve")
    );
}

BOOST_AUTO_TEST_CASE (peer_does_not_hang_when_resolve_throws)
{
  test_peer_does_not_hang_when_resolve_throws (boost::none);

  if (test_certificates)
  {
    test_peer_does_not_hang_when_resolve_throws (test_certificates);
  }
}

void test_peer_run_single (fhg::com::certificates_t const& certificates)
{
  using namespace fhg::com;
  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("12351")
                , certificates
                );
}

BOOST_AUTO_TEST_CASE (peer_run_single)
{
  test_peer_run_single (boost::none);

  if (test_certificates)
  {
    test_peer_run_single (test_certificates);
  }
}

namespace
{
  fhg::com::host_t host (boost::asio::ip::tcp::endpoint const& ep)
  {
    return fhg::com::host_t
      (fhg::util::connectable_to_address_string (ep.address()));
  }
  fhg::com::port_t port (boost::asio::ip::tcp::endpoint const& ep)
  {
    return fhg::com::port_t (std::to_string (ep.port()));
  }
}

void test_peer_run_two (fhg::com::certificates_t const& certificates)
{
  using namespace fhg::com;

  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                , certificates
                );

  peer_t peer_2 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                , certificates
                );

  peer_1.send ( peer_1.connect_to ( host (peer_2.local_endpoint())
                                  , port (peer_2.local_endpoint())
                                  )
              , "hello world!"
              );
    message_t m;
    peer_2.TESTING_ONLY_recv (&m);

  BOOST_CHECK_EQUAL (m.header.src, peer_1.address());
  BOOST_CHECK_EQUAL
    (std::string (m.data.begin(), m.data.end()), "hello world!");
}

BOOST_AUTO_TEST_CASE (peer_run_two)
{
  test_peer_run_two (boost::none);

  if (test_certificates)
  {
    test_peer_run_two (test_certificates);
  }
}

void test_resolve_peer_names (fhg::com::certificates_t const& certificates)
{
  using namespace fhg::com;

  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                , certificates
                );

  peer_t peer_2 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                , certificates
                );
}

BOOST_AUTO_TEST_CASE (resolve_peer_names)
{
  test_resolve_peer_names (boost::none);

  if (test_certificates)
  {
      test_resolve_peer_names (test_certificates);
  }
}


void test_peer_loopback (fhg::com::certificates_t const& certificates)
{
  using namespace fhg::com;

  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                , certificates
                );

  p2p::address_t const addr ( peer_1.connect_to ( host (peer_1.local_endpoint())
                                                , port (peer_1.local_endpoint())
                                                )
                            );

    for (std::size_t i (0); i < 10000; ++i)
    {
      peer_1.send(addr, "hello world!");
    }
}

BOOST_AUTO_TEST_CASE (peer_loopback)
{
  test_peer_loopback (boost::none);

  if (test_certificates)
  {
    test_peer_loopback (test_certificates);
  }
}

void test_send_to_nonexisting_peer (fhg::com::certificates_t const& certificates)
{
  using namespace fhg::com;

  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                , certificates
                );

  BOOST_CHECK_THROW ( peer_1.connect_to
                        (host_t ("unknown host"), port_t ("unknown service"))
                    , std::exception
                    );
}

BOOST_AUTO_TEST_CASE (send_to_nonexisting_peer)
{
  test_send_to_nonexisting_peer (boost::none);

  if (test_certificates)
  {
    test_send_to_nonexisting_peer (test_certificates);
  }
}

void test_send_large_data (fhg::com::certificates_t const& certificates)
{
  using namespace fhg::com;

  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                , certificates
                );

  peer_1.send( peer_1.connect_to ( host (peer_1.local_endpoint())
                                 , port (peer_1.local_endpoint())
                                 )
             , std::string (2<<25, 'X')
             );
    message_t r;
    peer_1.TESTING_ONLY_recv(&r);

    BOOST_CHECK_EQUAL(2<<25, r.data.size());
}

BOOST_AUTO_TEST_CASE (send_large_data)
{
  test_send_large_data (boost::none);

  if (test_certificates)
  {
      test_send_large_data (test_certificates);
  }
}

void test_peers_with_fixed_ports (fhg::com::certificates_t const& certificates)
{
  using namespace fhg::com;

  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                , certificates
                );

  peer_t peer_2 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                , certificates
                );

  peer_1.send( peer_1.connect_to ( host (peer_2.local_endpoint())
                                 , port (peer_2.local_endpoint())
                                 )
             , "hello world!"
             );
}

BOOST_AUTO_TEST_CASE (peers_with_fixed_ports)
{
  test_peers_with_fixed_ports (boost::none);
  test_peers_with_fixed_ports
    (boost::filesystem::path (boost::filesystem::current_path()/"certs"));
}

void test_peers_with_fixed_ports_reuse (fhg::com::certificates_t const& certificates)
{
  using namespace fhg::com;

  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                , certificates
                );

  peer_t peer_2 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                , certificates
                );

  peer_1.send ( peer_1.connect_to ( host (peer_2.local_endpoint())
                                  , port (peer_2.local_endpoint())
                                  )
              , "hello world!"
              );
}

BOOST_AUTO_TEST_CASE (peers_with_fixed_ports_reuse)
{
  test_peers_with_fixed_ports_reuse (boost::none);

  if (test_certificates)
  {
    test_peers_with_fixed_ports_reuse (test_certificates);
  }
}

void test_two_peers_one_restarts_repeatedly (fhg::com::certificates_t const& certificates)
{
  using namespace fhg::com;

  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                , certificates
                );

  bool stop_request (false);

  //! \note Race: Possibly taken. Needs to be known before peer
  //! starts, though, to allow reconnecting.
  boost::asio::ip::tcp::endpoint peer_2_endpoint
    (boost::asio::ip::address::from_string ("127.0.0.1"), 15123);

  std::thread sender ( [&peer_1, &stop_request, &peer_2_endpoint]
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
    peer_t peer_2
      ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
      , host (peer_2_endpoint), port (peer_2_endpoint), certificates
      );

    try
    {
      peer_2.send ( peer_2.connect_to ( host (peer_1.local_endpoint())
                                      , port (peer_1.local_endpoint())
                                      )
                  , "hello world!"
                  );
    }
    catch (boost::system::system_error const &se)
    {
      if (se.code ().value () != boost::asio::error::eof)
      {
        throw;
      }
    }
  }

  stop_request = true;

  sender.join ();
}

BOOST_AUTO_TEST_CASE (two_peers_one_restarts_repeatedly)
{
  test_two_peers_one_restarts_repeatedly (boost::none);

  if (test_certificates)
  {
    test_two_peers_one_restarts_repeatedly (test_certificates);
  }
}

BOOST_AUTO_TEST_CASE (invalid_certificates_directory)
{
  using namespace fhg::com;

  BOOST_REQUIRE_THROW
  (
    peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                  , host_t("localhost")
                  , port_t("0")
                  , boost::filesystem::path (fhg::util::testing::random_string())
                  );
  , std::exception
  );
}
