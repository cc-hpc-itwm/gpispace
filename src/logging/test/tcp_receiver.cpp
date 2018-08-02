#include <logging/tcp_receiver.hpp>
#include <logging/test/message.hpp>

#include <rpc/remote_function.hpp>
#include <rpc/remote_socket_endpoint.hpp>
#include <rpc/remote_tcp_endpoint.hpp>
#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
#include <rpc/service_socket_provider.hpp>
#include <rpc/service_tcp_provider.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/testing/printer/future.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace logging
  {
    BOOST_AUTO_TEST_CASE (receiver_shall_throw_if_connecting_fails)
    {
      std::promise<tcp_endpoint> received;
      auto received_future (received.get_future());

      util::testing::require_exception
        ( []
          {
            tcp_receiver const receiver
              (tcp_endpoint ("x.invalid", 0), [] (message const&) {});
          }
        , boost::system::system_error
            (boost::asio::error::host_not_found, "resolve")
        );
    }

    BOOST_AUTO_TEST_CASE (receiver_shall_register_with_emitter_on_construction)
    {
      std::promise<tcp_endpoint> received;
      auto received_future (received.get_future());

      rpc::service_dispatcher dispatcher;
      util::scoped_boost_asio_io_service_with_threads io_service (1);
      rpc::service_handler<protocol::register_tcp_receiver> const register_tcp
        ( dispatcher
        , [&] (tcp_endpoint const& endpoint)
          {
            received.set_value (endpoint);
          }
        );
      rpc::service_tcp_provider const provider (io_service, dispatcher);
      auto const endpoint
        (util::connectable_to_address_string (provider.local_endpoint()));

      tcp_receiver const receiver (endpoint, [] (message const&) {});

      BOOST_REQUIRE_EQUAL
        ( received_future.wait_for (std::chrono::milliseconds (200))
        , std::future_status::ready
        );

      //! \note `tcp_receiver` does not have an accessor for the
      //! endpoint used, but since this is a node-local test, the
      //! hostname at least should be the same, so check that in
      //! addition to the fact it registered at all.
      BOOST_REQUIRE_EQUAL (received_future.get().host, endpoint.first);
    }

    BOOST_AUTO_TEST_CASE (receiver_shall_call_callback_for_a_message)
    {
      std::promise<tcp_endpoint> registered;

      rpc::service_dispatcher dispatcher;
      util::scoped_boost_asio_io_service_with_threads io_service (1);
      rpc::service_handler<protocol::register_tcp_receiver> const register_tcp
        ( dispatcher
        , [&] (tcp_endpoint const& endpoint)
          {
            registered.set_value (endpoint);
          }
        );
      rpc::service_tcp_provider const provider (io_service, dispatcher);

      auto const sent (util::testing::random<message>{}());
      std::promise<message> received;
      auto received_future (received.get_future());

      tcp_receiver const receiver
        ( util::connectable_to_address_string (provider.local_endpoint())
        , [&] (message const& m)
          {
            received.set_value (m);
          }
        );

      auto const receiver_endpoint (registered.get_future().get());

      rpc::remote_tcp_endpoint emit_client (io_service, receiver_endpoint);
      rpc::sync_remote_function<protocol::receive> {emit_client} (sent);

      BOOST_REQUIRE_EQUAL
        ( received_future.wait_for (std::chrono::milliseconds (200))
        , std::future_status::ready
        );

      BOOST_REQUIRE_EQUAL (received_future.get(), sent);
    }
  }
}
