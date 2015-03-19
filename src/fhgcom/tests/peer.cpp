#define BOOST_TEST_MODULE PeerTest
#include <boost/test/unit_test.hpp>

#include <fhgcom/peer.hpp>
#include <fhgcom/peer_info.hpp>
#include <fhgcom/tests/address_printer.hpp>

#include <fhg/util/boost/asio/ip/address.hpp>
#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <fhg/util/random_string.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>

BOOST_AUTO_TEST_CASE (peer_run_single)
{
  using namespace fhg::com;
  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("1235")
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
    peer_2.recv (&m);

  BOOST_CHECK_EQUAL (m.header.src, peer_1.address());
  BOOST_CHECK_EQUAL
    (std::string (m.data.begin(), m.data.end()), "hello world!");
}

BOOST_AUTO_TEST_CASE (resolve_peer_names)
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
}

BOOST_AUTO_TEST_CASE (peer_loopback)
{
  using namespace fhg::com;

  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
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
    peer_1.recv(&r);

    BOOST_CHECK_EQUAL(2<<25, r.data.size());
}

BOOST_AUTO_TEST_CASE (peers_with_fixed_ports)
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

  peer_1.send( peer_1.connect_to ( host (peer_2.local_endpoint())
                                 , port (peer_2.local_endpoint())
                                 )
             , "hello world!"
             );
}

BOOST_AUTO_TEST_CASE (peers_with_fixed_ports_reuse)
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
}

BOOST_AUTO_TEST_CASE (two_peers_one_restarts_repeatedly)
{
  using namespace fhg::com;

  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                );

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
    peer_t peer_2
      ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
      , host (peer_2_endpoint), port (peer_2_endpoint)
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
