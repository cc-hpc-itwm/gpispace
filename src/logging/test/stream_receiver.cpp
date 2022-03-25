// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <logging/stream_receiver.hpp>
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
#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/printer/list.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

//! \todo src/util-generic/testing/printer/boost/system_error.hpp?
FHG_BOOST_TEST_LOG_VALUE_PRINTER (::boost::system::error_code, os, ec)
{
  os << ec << " (\""<< ec.message() << "\")";
}

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
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
              << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (value)
              << "] not in "
              << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (choices);
          return oss.str();
        }
      }

      //! \todo src/util-generic/testing/require_choice.hpp?
#define FHG_UTIL_TESTING_REQUIRE_EQUAL_ONE_OF(value_, choices_...)      \
      BOOST_REQUIRE_MESSAGE                                             \
        ( ::fhg::util::testing::detail::require_equal_one_of_condition  \
            (value_, {choices_})                                        \
        , ::fhg::util::testing::detail::require_equal_one_of_message    \
            (value_, #value_, {choices_})                               \
        )

      //! \todo src/util-generic/testing/require_strings.hpp?
#define FHG_UTIL_TESTING_REQUIRE_STARTS_WITH(value_, prefix_)           \
      BOOST_REQUIRE_EQUAL                                               \
        (std::string (value_).substr (0, strlen (prefix_)), prefix_)

    }
  }

  namespace logging
  {
    BOOST_AUTO_TEST_CASE (receiver_shall_throw_if_connecting_fails)
    {
      std::promise<endpoint> received;
      auto received_future (received.get_future());

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
          FHG_UTIL_TESTING_REQUIRE_STARTS_WITH (catched.what(), "resolve:");
          FHG_UTIL_TESTING_REQUIRE_EQUAL_ONE_OF
            ( catched.code()
            , ::boost::asio::error::host_not_found
            , ::boost::asio::error::host_not_found_try_again
            );
        }

      } comparator;

      util::testing::require_exception
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

      rpc::service_dispatcher dispatcher;
      util::scoped_boost_asio_io_service_with_threads io_service (1);
      rpc::service_handler<protocol::register_receiver> const register_handler
        ( dispatcher
        , [&] (endpoint const& endpoint)
          {
            received.set_value (endpoint);
          }
        , fhg::rpc::not_yielding
        );
      rpc::service_tcp_provider const provider (io_service, dispatcher);
      auto const endpoint
        (util::connectable_to_address_string (provider.local_endpoint()));

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

      rpc::service_dispatcher dispatcher;
      util::scoped_boost_asio_io_service_with_threads io_service (1);
      rpc::service_handler<protocol::register_receiver> const register_handler
        ( dispatcher
        , [&] (endpoint const& endpoint)
          {
            registered.set_value (endpoint);
          }
        , fhg::rpc::not_yielding
        );
      rpc::service_tcp_provider const provider (io_service, dispatcher);

      auto const sent (util::testing::random<message>{}());
      std::promise<message> received;
      auto received_future (received.get_future());

      stream_receiver const receiver
        ( tcp_endpoint
            (util::connectable_to_address_string (provider.local_endpoint()))
        , [&] (message const& m)
          {
            received.set_value (m);
          }
        );

      auto const receiver_endpoint (registered.get_future().get());

      rpc::remote_tcp_endpoint emit_client
        (io_service, *receiver_endpoint.as_tcp);
      rpc::sync_remote_function<protocol::receive> {emit_client} (sent);

      BOOST_REQUIRE_EQUAL
        ( received_future.wait_for (std::chrono::milliseconds (200))
        , std::future_status::ready
        );

      BOOST_REQUIRE_EQUAL (received_future.get(), sent);
    }
  }
}
