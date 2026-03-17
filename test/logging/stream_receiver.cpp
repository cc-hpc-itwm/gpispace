// Copyright (C) 2018-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/logging/stream_receiver.hpp>
#include <test/logging/message.hpp>

#include <gspc/rpc/remote_function.hpp>
#include <gspc/rpc/remote_socket_endpoint.hpp>
#include <gspc/rpc/remote_tcp_endpoint.hpp>
#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_socket_provider.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/testing/printer/future.hpp>
#include <gspc/testing/printer/generic.hpp>
#include <gspc/testing/printer/list.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <utility>

//! \todo src/util-generic/testing/printer/system_error.hpp?
GSPC_BOOST_TEST_LOG_VALUE_PRINTER (std::error_code, os, ec)
{
  os << ec << " (\""<< ec.message() << "\")";
}

namespace gspc::testing::detail
{
  template<typename Value>
    bool require_equal_one_of_condition
      (Value const& value, std::list<Value> const& choices)
  {
    return std::any_of ( choices.begin(), choices.end()
                       , [&] (Value const& choice)
                         {
                           return value == choice;
                         }
                       );
  }

  template<typename Value>
    std::string require_equal_one_of_message
      ( Value const& value
      , char const* const variable
      , std::list<Value> const& choices
      )
  {
    std::ostringstream oss;
    oss << variable
        << " ["
        << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (value)
        << "] not in "
        << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (choices);
    return oss.str();
  }
}

//! \todo src/util-generic/testing/require_choice.hpp?
#define GSPC_TESTING_REQUIRE_EQUAL_ONE_OF(value_, choices_...)         \
  BOOST_REQUIRE_MESSAGE                                                \
    ( ::gspc::testing::detail::require_equal_one_of_condition          \
        (value_, {choices_})                                           \
    , ::gspc::testing::detail::require_equal_one_of_message            \
        (value_, #value_, {choices_})                                  \
    )

//! \todo src/util-generic/testing/require_strings.hpp?
#define GSPC_TESTING_REQUIRE_STARTS_WITH(value_, prefix_)              \
  BOOST_REQUIRE_EQUAL                                                  \
    (std::string (value_).substr (0, strlen (prefix_)), prefix_)

namespace gspc::logging
{
  BOOST_AUTO_TEST_CASE (receiver_shall_throw_if_connecting_fails)
  {
    std::promise<endpoint> received;
    std::ignore = received.get_future();

    //! \note 'x.invalid' will always be not found as per RFC 6761
    //! 6.4, yet the response isn't always authorative, thus may be
    //! both `host_not_found` or `host_not_found_try_again`. Thus,
    //! handle them both with a custom comparator. We ignore lhs and
    //! only use that to get the right type catched.
    struct
    {
      void operator() ( ::boost::system::system_error const&
                      , ::boost::system::system_error const& catched
                      )
      {
        GSPC_TESTING_REQUIRE_STARTS_WITH (catched.what(), "resolve:");
        GSPC_TESTING_REQUIRE_EQUAL_ONE_OF
          ( catched.code()
          , ::boost::asio::error::host_not_found
          , ::boost::asio::error::host_not_found_try_again
          );
      }

    } comparator;

    gspc::testing::require_exception
      ( []
        {
          stream_receiver const receiver
            (tcp_endpoint ("x.invalid", 0), [] (message const&) {});
        }
      , ::boost::system::system_error
          (::boost::asio::error::host_not_found, "resolve")
      , comparator
      );
  }

  BOOST_AUTO_TEST_CASE (receiver_shall_register_with_emitter_on_construction)
  {
    std::promise<endpoint> received;
    auto received_future (received.get_future());

    gspc::rpc::service_dispatcher dispatcher;
    gspc::util::scoped_boost_asio_io_service_with_threads io_service (1);
    gspc::rpc::service_handler<protocol::register_receiver> const register_handler
      ( dispatcher
      , [&] (endpoint const& endpoint)
        {
          received.set_value (endpoint);
        }
      , gspc::rpc::not_yielding
      );
    gspc::rpc::service_tcp_provider const provider (io_service, dispatcher);
    auto const endpoint
      (gspc::util::connectable_to_address_string (provider.local_endpoint()));

    stream_receiver const receiver
      (tcp_endpoint (endpoint), [] (message const&) {});

    BOOST_REQUIRE_EQUAL
      ( received_future.wait_for (std::chrono::milliseconds (200))
      , std::future_status::ready
      );

    //! \note `stream_receiver` does not have an accessor for the
    //! endpoint used, but since this is a node-local test, the
    //! hostname at least should be the same, so check that in
    //! addition to the fact it registered at all.
    BOOST_REQUIRE_EQUAL (received_future.get().as_tcp->host, endpoint.first);
  }

  BOOST_AUTO_TEST_CASE (receiver_shall_call_callback_for_a_message)
  {
    std::promise<endpoint> registered;

    gspc::rpc::service_dispatcher dispatcher;
    gspc::util::scoped_boost_asio_io_service_with_threads io_service (1);
    gspc::rpc::service_handler<protocol::register_receiver> const register_handler
      ( dispatcher
      , [&] (endpoint const& endpoint)
        {
          registered.set_value (endpoint);
        }
      , gspc::rpc::not_yielding
      );
    gspc::rpc::service_tcp_provider const provider (io_service, dispatcher);

    auto const sent (gspc::testing::random<message>{}());
    std::promise<message> received;
    auto received_future (received.get_future());

    stream_receiver const receiver
      ( tcp_endpoint
          (gspc::util::connectable_to_address_string (provider.local_endpoint()))
      , [&] (message const& m)
        {
          received.set_value (m);
        }
      );

    auto const receiver_endpoint (registered.get_future().get());

    gspc::rpc::remote_tcp_endpoint emit_client
      (io_service, *receiver_endpoint.as_tcp);
    gspc::rpc::sync_remote_function<protocol::receive> {emit_client} (sent);

    BOOST_REQUIRE_EQUAL
      ( received_future.wait_for (std::chrono::milliseconds (200))
      , std::future_status::ready
      );

    BOOST_REQUIRE_EQUAL (received_future.get(), sent);
  }
}
