// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <logging/stream_emitter.hpp>
#include <logging/test/message.hpp>

#include <util-rpc/remote_function.hpp>
#include <util-rpc/remote_socket_endpoint.hpp>
#include <util-rpc/remote_tcp_endpoint.hpp>
#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>
#include <util-rpc/service_socket_provider.hpp>
#include <util-rpc/service_tcp_provider.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/testing/printer/future.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/mpl/list.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace logging
  {
    namespace
    {
      enum stream_protocol
      {
        socket,
        tcp,
      };

      template<stream_protocol> struct emitter_endpoint;
      template<> struct emitter_endpoint<stream_protocol::socket>
      {
        static socket_endpoint::Socket endpoint (stream_emitter const& emitter)
        {
          return emitter.local_endpoint().as_socket->socket;
        }
      };
      template<> struct emitter_endpoint<stream_protocol::tcp>
      {
        static tcp_endpoint endpoint (stream_emitter const& emitter)
        {
          return *emitter.local_endpoint().as_tcp;
        }
      };

      template<stream_protocol receiver, stream_protocol registration>
        struct combination_t : emitter_endpoint<registration>
      {
        using receiver_type
          = typename std::conditional < receiver == stream_protocol::tcp
                                      , rpc::service_tcp_provider
                                      , rpc::service_socket_provider
                                      >::type;

        using registration_method
          = typename std::conditional < registration == stream_protocol::tcp
                                      , rpc::remote_tcp_endpoint
                                      , rpc::remote_socket_endpoint
                                      >::type;
      };

      socket_endpoint get_endpoint (rpc::service_socket_provider const& receiver)
      {
        return receiver.local_endpoint();
      }
      tcp_endpoint get_endpoint (rpc::service_tcp_provider const& receiver)
      {
        return util::connectable_to_address_string (receiver.local_endpoint());
      }

      using all_combinations = ::boost::mpl::list
        < combination_t<stream_protocol::tcp, stream_protocol::tcp>
        , combination_t<stream_protocol::tcp, stream_protocol::socket>
        , combination_t<stream_protocol::socket, stream_protocol::tcp>
        , combination_t<stream_protocol::socket, stream_protocol::socket>
        >;
    }

    BOOST_AUTO_TEST_CASE_TEMPLATE ( emitter_calls_rpc_with_given_combination
                                  , Combination
                                  , all_combinations
                                  )
    {
      auto const sent (util::testing::random<message>{}());

      stream_emitter emitter;

      std::promise<message> received;
      auto received_future (received.get_future());

      rpc::service_dispatcher service_dispatcher;
      util::scoped_boost_asio_io_service_with_threads io_service (2);
      rpc::service_handler<protocol::receive> const receive
        ( service_dispatcher
        , [&] (message const& m)
          {
            received.set_value (m);
          }
        , fhg::rpc::not_yielding
        );

      typename Combination::receiver_type const service_provider
        (io_service, service_dispatcher);

      typename Combination::registration_method client
        (io_service, Combination::endpoint (emitter));
      rpc::sync_remote_function<protocol::register_receiver>
        {client} (get_endpoint (service_provider));

      emitter.emit_message (sent);

      BOOST_REQUIRE_EQUAL
        ( received_future.wait_for (std::chrono::milliseconds (200))
        , std::future_status::ready
        );

      BOOST_REQUIRE_EQUAL (received_future.get(), sent);
    }
  }
}
