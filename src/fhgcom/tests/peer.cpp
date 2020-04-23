#include <fhgcom/peer.hpp>
#include <fhgcom/peer_info.hpp>
#include <fhgcom/tests/address_printer.hpp>

#include <test/certificates_data.hpp>
#include <test/hopefully_free_port.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/finally.hpp>
#include <util-generic/functor_visitor.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/printer/vector.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <atomic>
#include <cstddef>
#include <exception>
#include <functional>
#include <mutex>
#include <ostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

BOOST_TEST_DECORATOR (*boost::unit_test::timeout (2))
BOOST_DATA_TEST_CASE
  (peer_does_not_hang_when_resolve_throws, certificates_data, certificates)
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

BOOST_DATA_TEST_CASE (constructing_one_works, certificates_data, certificates)
{
  using namespace fhg::com;
  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("12351")
                , certificates
                );
}

BOOST_DATA_TEST_CASE (constructing_two_works, certificates_data, certificates)
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

  enum class Order
  {
    AsyncRecv_Connect_Send,
    Connect_Send_AsyncRecv,
    Connect_AsyncRecv_Send,
  };
  std::ostream& operator<< (std::ostream& os, Order const& o)
  {
    return os << ( o == Order::AsyncRecv_Connect_Send ? "AsyncRecv_Connect_Send"
                 : o == Order::Connect_Send_AsyncRecv ? "Connect_Send_AsyncRecv"
                 : o == Order::Connect_AsyncRecv_Send ? "Connect_AsyncRecv_Send"
                 : throw std::invalid_argument ("bad order")
                 );
  }

  std::vector<std::string> test_messages()
  {
    return { "hello world!"
           , std::string (2 << 25, 'X')
           };
  }
  std::vector<Order> operation_orders()
  {
    return { Order::AsyncRecv_Connect_Send
           , Order::Connect_Send_AsyncRecv
           , Order::Connect_AsyncRecv_Send
           };
  }
}

BOOST_DATA_TEST_CASE ( peer_run_two
                     , certificates_data * test_messages() * operation_orders()
                     , certificates
                     , message
                     , order
                     )
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

  fhg::util::thread::event<> recv_finished;
  boost::system::error_code error;
  message_t m;
  boost::optional<p2p::address_t> connection;

  auto const async_recv
    ( [&]
      {
        peer_2.async_recv ( [&] ( boost::system::error_code ec
                                , boost::optional<fhg::com::p2p::address_t>
                                , message_t message
                                )
                            {
                              error = ec;
                              m = std::move (message);
                              recv_finished.notify();
                            }
                          );
      }
    );
  auto const connect
    ( [&]
      {
        connection = peer_1.connect_to ( host (peer_2.local_endpoint())
                                       , port (peer_2.local_endpoint())
                                       );
      }
    );
  auto const send
    ( [&]
      {
        peer_1.send (connection.get(), message);
      }
    );

  switch (order)
  {
  case Order::AsyncRecv_Connect_Send:
    async_recv();
    connect();
    send();
    recv_finished.wait();
    break;
  case Order::Connect_Send_AsyncRecv:
    connect();
    send();
    async_recv();
    recv_finished.wait();
    break;
  case Order::Connect_AsyncRecv_Send:
    connect();
    async_recv();
    send();
    recv_finished.wait();
    break;
  }

  BOOST_REQUIRE_EQUAL (error, boost::system::errc::success);

  BOOST_CHECK_EQUAL (m.header.src, peer_1.address());
  BOOST_CHECK_EQUAL (std::string (m.data.begin(), m.data.end()), message);
}

BOOST_DATA_TEST_CASE (peer_loopback_forbidden, certificates_data, certificates)
{
  using namespace fhg::com;

  fhg::com::peer_t peer_1
    ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    , certificates
    );

  fhg::util::testing::require_exception
    ( [&]
      {
        peer_1.connect_to ( fhg::com::host_t (fhg::util::hostname())
                          , port (peer_1.local_endpoint())
                          );
      }
    , std::logic_error ("unable to connect to self")
    );
}

BOOST_DATA_TEST_CASE
  (send_to_nonexisting_peer, certificates_data, certificates)
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

BOOST_DATA_TEST_CASE
  (two_peers_one_restarts_repeatedly, certificates_data, certificates)
{
  using namespace fhg::com;

  peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , host_t("localhost")
                , port_t("0")
                , certificates
                );

  bool stop_request (false);

  test::hopefully_free_port peer_2_port;
  fhg::com::port_t peer_2_port_string (std::to_string (peer_2_port));

  std::thread sender ( [&peer_1, &stop_request, &peer_2_port_string]
                       {
                         while (not stop_request)
                         {
                           try
                           {
                             peer_1.send ( peer_1.connect_to
                                             ( host_t ("localhost")
                                             , peer_2_port_string
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

  peer_2_port.release();
  for (std::size_t i (0); i < 100; ++i)
  {
    peer_t peer_2
      ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
      , host_t ("localhost"), peer_2_port_string, certificates
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

namespace
{
  // Not defined in early versions of Boost.Asio, but bumping just for
  // that enum would be kind of absurd. This is wrapped version of
  // OpenSSL's SSL_R_SHORT_READ.
  boost::system::error_code const boost_asio_ssl_error_stream_truncated
    (0x140000db, boost::asio::error::get_ssl_category());
}

// A race in destruction of peer_t, while a message was just sent,
// resulted in
//
// - corrupted messages being received (assertion fails)
// - segfaults in various places (e.g. SSL context destruction)
//   (signal raised)
// - 'header/data length mismatch' when the destructing peer was still
//   trying to send (terminate due to being thrown in some thread)
//
// All of the above are not easily triggered by anything in
// particular, so this test tries to reproduce them by doing the
// construct-connect-send-destruct sequence multiple times. The
// repetition counter was adjusted to take less than 30 seconds on LTS
// while still reproducing every few runs.
BOOST_DATA_TEST_CASE
  ( destruction_right_after_sending_should_not_hangsegfaultcrashcorruptdata
  , certificates_data
  , certificates
  )
{
  auto const payload (fhg::util::testing::random<std::string>{}());

  using Received
    = boost::variant<boost::system::error_code, fhg::com::message_t>;

  // No lock: async_recv loop is single threaded.
  std::vector<Received> receive_results;
  std::mutex send_results_guard;
  std::vector<boost::system::error_code> send_results;

  {
    std::atomic<bool> parent_destructing (false);

    std::function< void ( boost::system::error_code
                        , boost::optional<fhg::com::p2p::address_t>
                        , fhg::com::message_t
                        )
                 > record_receive;

    fhg::com::peer_t parent
      ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
      , fhg::com::host_t ("localhost")
      , fhg::com::port_t ("0")
      , certificates
      );
    FHG_UTIL_FINALLY ([&] { parent_destructing = true; });

    record_receive
      = [&] ( boost::system::error_code ec
            , boost::optional<fhg::com::p2p::address_t>
            , fhg::com::message_t message
            )
        {
          receive_results.push_back (ec ? Received (ec) : Received (message));

          if (!parent_destructing)
          {
            parent.async_recv (record_receive);
          }
        };
    parent.async_recv (record_receive);

    int repetitions (10000);
    while (repetitions --> 0)
    {
      fhg::com::peer_t child
        ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
        , fhg::com::host_t ("localhost")
        , fhg::com::port_t ("0")
        , certificates
        );

      child.async_send
        ( child.connect_to
            (host (parent.local_endpoint()), port (parent.local_endpoint()))
        , payload
        , [&] (boost::system::error_code ec)
          {
            std::lock_guard<std::mutex> const lock (send_results_guard);
            send_results.emplace_back (ec);
          }
        );

      // Bug happens here: child is going to be destructed, while send
      // is still in flight.
    }
  }

  for (auto const& ec : send_results)
  {
    BOOST_CHECK_MESSAGE
      ( ec == boost::system::error_code{}
     || ec == boost::system::errc::operation_canceled
      , ec << " shall be one of [success (system.0) generic.operation_canceled]"
      );
  }
  for (auto const& result : receive_results)
  {
    fhg::util::visit<void>
      ( result
      , [] (boost::system::error_code const& ec)
        {
          BOOST_CHECK_MESSAGE
            ( ec == boost::asio::error::misc_errors::eof
           || ec == boost::system::errc::operation_canceled
           || ec == boost_asio_ssl_error_stream_truncated
            , ec
            << " shall be one of [asio.misc.eof, generic.operation_canceled, "
               "asio.ssl.stream_truncated]"
            );
        }
      , [&] (fhg::com::message_t const& message)
        {
          BOOST_CHECK_EQUAL (message.header.type_of_msg, 0);
          BOOST_CHECK_EQUAL (message.header.length, payload.size());
          BOOST_CHECK_EQUAL
            (message.data, std::vector<char> (payload.begin(), payload.end()));
        }
      );
  }
}

BOOST_AUTO_TEST_CASE (require_certificates_location_to_exist)
{
  using namespace fhg::com;

  fhg::util::temporary_path const empty_directory;

  BOOST_REQUIRE_EXCEPTION
    ( peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                    , host_t ("localhost")
                    , port_t ("0")
                    , fhg::com::Certificates
                        {boost::filesystem::path (empty_directory) / "nonexist"}
                    )
    , boost::filesystem::filesystem_error
    , [&] (boost::filesystem::filesystem_error const& exc)
      {
        return std::string (exc.what()).find
          ("No such file or directory") != std::string::npos;
      }
    );
}

BOOST_AUTO_TEST_CASE
  (when_location_exists_require_to_contain_certificate_files)
{
  using namespace fhg::com;

  fhg::util::temporary_path const empty_directory;

  BOOST_REQUIRE_EXCEPTION
    ( peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                    , host_t ("localhost")
                    , port_t ("0")
                    , fhg::com::Certificates {empty_directory}
                    )
    , boost::filesystem::filesystem_error
    , [&] (boost::filesystem::filesystem_error const& exc)
      {
        return std::string (exc.what()).find
          ("No such file or directory") != std::string::npos;
      }
    );
}

BOOST_AUTO_TEST_CASE (using_empty_Certificates_throws)
{
  using namespace fhg::com;

  fhg::util::temporary_path const certificates_directory;

  fhg::util::temporary_path const crt
    (boost::filesystem::path (certificates_directory) / "server.crt");

  fhg::util::temporary_path const key
    (boost::filesystem::path (certificates_directory) / "server.key");

  BOOST_REQUIRE_EXCEPTION
    ( peer_t peer_1 ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
                    , host_t ("localhost")
                    , port_t ("0")
                    , fhg::com::Certificates {certificates_directory}
                    )
    , std::runtime_error
    , [&] (std::runtime_error const& exc)
      {
        return std::string (exc.what()).find
          ("use_certificate_chain_file: no start line") != std::string::npos;
      }
    );
}

BOOST_TEST_DECORATOR (*boost::unit_test::timeout (2))
BOOST_AUTO_TEST_CASE
  (client_peer_tries_to_connect_to_secure_peer_using_tcp)
{
  using namespace fhg::com;

  peer_t peer_1
    ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , host_t ("localhost")
    , port_t ("0")
    , gspc::testing::no_certs()
    );

  peer_t peer_2
    ( fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , host_t ("localhost")
    , port_t ("0")
    , gspc::testing::yes_certs()
    );

  BOOST_REQUIRE_THROW
  ( { peer_1.connect_to ( host (peer_2.local_endpoint())
                        , port (peer_2.local_endpoint())
                        );
      while (peer_2.TESTING_ONLY_handshake_exception() == nullptr);
      std::rethrow_exception (peer_2.TESTING_ONLY_handshake_exception());
    }
  , fhg::com::handshake_exception
  );
}
