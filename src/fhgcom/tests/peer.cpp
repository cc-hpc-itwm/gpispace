// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <fhgcom/peer.hpp>
#include <fhgcom/channel.hpp>
#include <fhgcom/peer_info.hpp>

#include <testing/certificates_data.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/finally.hpp>
#include <util-generic/functor_visitor.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/printer/vector.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

// should only need ssl/context.hpp, but that's missing an include
#include <boost/asio/ssl.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/version.hpp>

#include <atomic>
#include <cstddef>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_DATA_TEST_CASE
  (peer_does_not_hang_when_resolve_throws, certificates_data, certificates)
{
  fhg::util::testing::require_exception
    ( [&certificates]
      {
        fhg::com::peer_t ( std::make_unique<::boost::asio::io_service>()
                         , fhg::com::host_t ("NONONONONONONONONONO")
                         , fhg::com::port_t (0)
                         , certificates
                         );
      }
    , ::boost::system::system_error
        (::boost::asio::error::host_not_found, "resolve")
    );
}

BOOST_DATA_TEST_CASE (constructing_one_works, certificates_data, certificates)
{
  using namespace fhg::com;
  peer_t peer_1 ( std::make_unique<::boost::asio::io_service>()
                , host_t("localhost")
                , port_t(12351)
                , certificates
                );
}

BOOST_DATA_TEST_CASE (constructing_two_works, certificates_data, certificates)
{
  using namespace fhg::com;

  peer_t peer_1 ( std::make_unique<::boost::asio::io_service>()
                , host_t("localhost")
                , port_t(0)
                , certificates
                );

  peer_t peer_2 ( std::make_unique<::boost::asio::io_service>()
                , host_t("localhost")
                , port_t(0)
                , certificates
                );
}

namespace
{
  fhg::com::host_t host (::boost::asio::ip::tcp::endpoint const& ep)
  {
    return fhg::com::host_t
      (fhg::util::connectable_to_address_string (ep.address()));
  }
  fhg::com::port_t port (::boost::asio::ip::tcp::endpoint const& ep)
  {
    return fhg::com::port_t (ep.port());
  }

  enum class Order
  {
    AsyncRecv_Connect_Send,
    Connect_Send_AsyncRecv,
    Connect_AsyncRecv_Send,
  };
  std::ostream& operator<< (std::ostream& os, Order const& o)
  {
    switch (o)
    {
      case Order::AsyncRecv_Connect_Send:
        os << "AsyncRecv_Connect_Send";
        break;
      case Order::Connect_Send_AsyncRecv:
        os << "Connect_Send_AsyncRecv";
        break;
      case Order::Connect_AsyncRecv_Send:
        os << "Connect_AsyncRecv_Send";
        break;
    }

    return os;
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

  peer_t peer_1 ( std::make_unique<::boost::asio::io_service>()
                , host_t("localhost")
                , port_t(0)
                , certificates
                );

  peer_t peer_2 ( std::make_unique<::boost::asio::io_service>()
                , host_t("localhost")
                , port_t(0)
                , certificates
                );

  std::promise<fhg::com::peer_t::Received> recv_finished;

  auto const async_recv
    ( [&]
      {
        peer_2.async_recv ( [&] (auto received)
                            {
                              recv_finished.set_value (std::move (received));
                            }
                          );
      }
    );

  ::boost::optional<p2p::address_t> connection;
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

  auto const received
  { [&]
    {
  switch (order)
  {
  case Order::AsyncRecv_Connect_Send:
    async_recv();
    connect();
    send();
    break;
  case Order::Connect_Send_AsyncRecv:
    connect();
    send();
    async_recv();
    break;
  case Order::Connect_AsyncRecv_Send:
    connect();
    async_recv();
    send();
    break;
  }
  return recv_finished.get_future().get();
    }()
  };

  auto const& error (received.ec());
  BOOST_REQUIRE_EQUAL (error, ::boost::system::errc::success);

  auto const& m (received.message());
  BOOST_CHECK (m.header.src == peer_1.address());
  BOOST_CHECK_EQUAL (std::string (m.data.begin(), m.data.end()), message);
}

namespace fhg
{
  namespace com
  {
    BOOST_DATA_TEST_CASE ( channel_other_end_is_other_end
                         , certificates_data
                         , certificates
                         )
    {
      peer_t const a ( std::make_unique<::boost::asio::io_service>()
                     , host_t ("localhost")
                     , port_t (0)
                     , certificates
                     );
      auto const lp (a.local_endpoint());
      auto const h (host (lp));
      auto const p (port (lp));

      channel const b ( std::make_unique<::boost::asio::io_service>()
                      , port_t (0)
                      , certificates
                      , h
                      , p
                      );

      BOOST_CHECK (b.other_end() == p2p::address_t (h, p));
    }

    BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
    BOOST_DATA_TEST_CASE ( channel_sends_to_other_end
                         , certificates_data * test_messages()
                         , certificates
                         , message
                         )
    {
      peer_t a ( std::make_unique<::boost::asio::io_service>()
               , host_t ("localhost")
               , port_t (0)
               , certificates
               );
      auto const lp (a.local_endpoint());

      channel b ( std::make_unique<::boost::asio::io_service>()
                , port_t (0)
                , certificates
                , host (lp)
                , port (lp)
                );

      std::promise<fhg::com::peer_t::Received> recv_finished;

      a.async_recv
        ( [&] (auto received)
          {
            recv_finished.set_value (std::move (received));
          }
        );

      b.send (message);

      auto const received {recv_finished.get_future().get()};

      BOOST_REQUIRE_EQUAL (received.ec(), ::boost::system::errc::success);

      auto const& m (received.message());
      BOOST_CHECK (m.header.src == b.address());
      BOOST_CHECK_EQUAL (std::string (m.data.begin(), m.data.end()), message);
      BOOST_CHECK (received.source() == b.address());
    }
  }
}

BOOST_DATA_TEST_CASE (peer_loopback_forbidden, certificates_data, certificates)
{
  using namespace fhg::com;

  fhg::com::peer_t peer_1
    ( std::make_unique<::boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t (0)
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

  peer_t peer_1 ( std::make_unique<::boost::asio::io_service>()
                , host_t("localhost")
                , port_t(0)
                , certificates
                );

  BOOST_CHECK_THROW ( peer_1.connect_to
                        (host_t ("unknown host"), port_t (0))
                    , std::exception
                    );
}

namespace
{
  // Not defined in early versions of Boost.Asio, but bumping just for
  // that enum would be kind of absurd. This is wrapped version of
  // OpenSSL's SSL_R_SHORT_READ.
#if BOOST_VERSION <= 106100 // 1.61.0
  ::boost::system::error_code const boost_asio_ssl_error_stream_truncated
    (0x140000db, ::boost::asio::error::get_ssl_category());
#else
  auto const boost_asio_ssl_error_stream_truncated
    (::boost::asio::ssl::error::stream_truncated);
#endif
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
    = ::boost::variant<::boost::system::error_code, fhg::com::message_t>;

  // No lock: async_recv loop is single threaded.
  std::vector<Received> receive_results;
  std::mutex send_results_guard;
  std::vector<::boost::system::error_code> send_results;

  {
    std::atomic<bool> parent_destructing (false);

    std::function<void (fhg::com::peer_t::Received)> record_receive;

    fhg::com::peer_t parent
      ( std::make_unique<::boost::asio::io_service>()
      , fhg::com::host_t ("localhost")
      , fhg::com::port_t (0)
      , certificates
      );
    FHG_UTIL_FINALLY ([&] { parent_destructing = true; });

    record_receive
      = [&] (auto received)
        {
          receive_results.push_back
            (received.ec() ? Received (received.ec())
                           : Received (received.message())
            );

          if (!parent_destructing)
          {
            parent.async_recv (record_receive);
          }
        };
    parent.async_recv (record_receive);

    int repetitions (1000);
    while (repetitions --> 0)
    {
      fhg::com::peer_t child
        ( std::make_unique<::boost::asio::io_service>()
        , fhg::com::host_t ("localhost")
        , fhg::com::port_t (0)
        , certificates
        );

      child.async_send
        ( child.connect_to
            (host (parent.local_endpoint()), port (parent.local_endpoint()))
        , payload
        , [&] (::boost::system::error_code ec)
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
      ( ec == ::boost::system::error_code{}
     || ec == ::boost::system::errc::operation_canceled
      , ec << " shall be one of [success (system.0) generic.operation_canceled]"
      );
  }
  for (auto const& result : receive_results)
  {
    fhg::util::visit<void>
      ( result
      , [] (::boost::system::error_code const& ec)
        {
          BOOST_CHECK_MESSAGE
            ( ec == ::boost::asio::error::misc_errors::eof
           || ec == ::boost::system::errc::operation_canceled
           || ec == ::boost::system::errc::network_down
           || ec == boost_asio_ssl_error_stream_truncated
            , ec
            << " shall be one of [asio.misc.eof, generic.operation_canceled, "
               ", generic::network_down, asio.ssl.stream_truncated]"
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
    ( peer_t peer_1 ( std::make_unique<::boost::asio::io_service>()
                    , host_t ("localhost")
                    , port_t (0)
                    , fhg::com::Certificates
                        {::boost::filesystem::path (empty_directory) / "nonexist"}
                    )
    , ::boost::filesystem::filesystem_error
    , [&] (::boost::filesystem::filesystem_error const& exc)
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
    ( peer_t peer_1 ( std::make_unique<::boost::asio::io_service>()
                    , host_t ("localhost")
                    , port_t (0)
                    , fhg::com::Certificates {empty_directory}
                    )
    , ::boost::filesystem::filesystem_error
    , [&] (::boost::filesystem::filesystem_error const& exc)
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
    (::boost::filesystem::path (certificates_directory) / "server.crt");

  fhg::util::temporary_path const key
    (::boost::filesystem::path (certificates_directory) / "server.key");

  BOOST_REQUIRE_EXCEPTION
    ( peer_t peer_1 ( std::make_unique<::boost::asio::io_service>()
                    , host_t ("localhost")
                    , port_t (0)
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

BOOST_TEST_DECORATOR (*::boost::unit_test::timeout (30))
BOOST_AUTO_TEST_CASE
  (client_peer_tries_to_connect_to_secure_peer_using_tcp)
{
  using namespace fhg::com;

  peer_t peer_1
    ( std::make_unique<::boost::asio::io_service>()
    , host_t ("localhost")
    , port_t (0)
    , gspc::testing::no_certs()
    );

  peer_t peer_2
    ( std::make_unique<::boost::asio::io_service>()
    , host_t ("localhost")
    , port_t (0)
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
