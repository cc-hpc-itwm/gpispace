#include <boost/test/unit_test.hpp>

#include <fhgcom/peer.hpp>
#include <fhgcom/peer_info.hpp>
#include <fhgcom/tests/address_printer.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random_string.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>

BOOST_TEST_DECORATOR (*boost::unit_test::timeout (2))
BOOST_AUTO_TEST_CASE (peer_does_not_hang_when_resolve_throws)
{
  fhg::util::testing::require_exception
    ( []
      {
        fhg::com::peer_t ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                         , fhg::com::host_t ("NONONONONONONONONONO")
                         , fhg::com::port_t ("NONONONONONONONONONO")
                         );
      }
    , boost::system::system_error
        (boost::asio::error::service_not_found, "resolve")
    );
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

BOOST_AUTO_TEST_CASE (peer_run_two)
{
  using namespace fhg::com;

  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                );

  peer_t peer_2 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
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

BOOST_AUTO_TEST_CASE (send_to_nonexisting_peer)
{
  using namespace fhg::com;

  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                );

  BOOST_CHECK_THROW ( peer_1.connect_to
                        (host_t ("unknown host"), port_t ("unknown service"))
                    , std::exception
                    );
}

BOOST_AUTO_TEST_CASE (send_large_data)
{
  using namespace fhg::com;

  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
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
