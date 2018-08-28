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
#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/printer/list.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

//! \todo src/util-generic/testing/printer/boost/system_error.hpp?
FHG_BOOST_TEST_LOG_VALUE_PRINTER (boost::system::error_code, os, ec)
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
      std::promise<tcp_endpoint> received;
      auto received_future (received.get_future());

      //! \note 'x.invalid' will always be not found as per RFC 6761
      //! 6.4, yet the response isn't always authorative, thus may be
      //! both `host_not_found` or `host_not_found_try_again`. Thus,
      //! handle them both with a custom comparator. We ignore lhs and
      //! only use that to get the right type catched.
      struct
      {
        void operator() ( boost::system::system_error const&
                        , boost::system::system_error const& catched
                        )
        {
          FHG_UTIL_TESTING_REQUIRE_STARTS_WITH (catched.what(), "resolve:");
          FHG_UTIL_TESTING_REQUIRE_EQUAL_ONE_OF
            ( catched.code()
            , boost::asio::error::host_not_found
            , boost::asio::error::host_not_found_try_again
            );
        }

      } comparator;

      util::testing::require_exception
        ( []
          {
            tcp_receiver const receiver
              (tcp_endpoint ("x.invalid", 0), [] (message const&) {});
          }
        , boost::system::system_error
            (boost::asio::error::host_not_found, "resolve")
        , comparator
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
