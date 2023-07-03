// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-rpc/remote_function.hpp>
#include <util-rpc/remote_tcp_endpoint.hpp>
#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>
#include <util-rpc/service_tcp_provider.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/measure_average_time.hpp>
#include <util-generic/testing/printer/chrono.hpp>

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

namespace protocol
{
  FHG_RPC_FUNCTION_DESCRIPTION (get_blob, std::vector<char>());
  FHG_RPC_FUNCTION_DESCRIPTION (put_blob, void (std::vector<char>));
  FHG_RPC_FUNCTION_DESCRIPTION (echo_blob, std::vector<char> (std::vector<char>));
  FHG_RPC_FUNCTION_DESCRIPTION (nop, void());
}

#define service_and_client_setup_with_blob_size(size)                     \
  std::vector<char> blob (size);                                          \
                                                                          \
  fhg::util::scoped_boost_asio_io_service_with_threads io_service {2};    \
  fhg::rpc::service_dispatcher service_dispatcher;                        \
                                                                          \
  fhg::rpc::service_handler<protocol::get_blob> get_blob_service          \
    {service_dispatcher, [&] { return blob; }};                           \
  fhg::rpc::service_handler<protocol::put_blob> put_blob_service          \
    {service_dispatcher, [] (std::vector<char>) {}};                      \
  fhg::rpc::service_handler<protocol::echo_blob> echo_blob_service        \
    {service_dispatcher, [] (std::vector<char> b) { return b; }};         \
                                                                          \
  fhg::rpc::service_tcp_provider const server                             \
    {io_service, service_dispatcher};                                     \
  fhg::rpc::remote_tcp_endpoint endpoint                                  \
    { io_service                                                          \
    , fhg::util::connectable_to_address_string (server.local_endpoint())  \
    };                                                                    \
                                                                          \
  fhg::rpc::sync_remote_function<protocol::get_blob> get_blob {endpoint}; \
  fhg::rpc::sync_remote_function<protocol::put_blob> put_blob {endpoint}; \
  fhg::rpc::sync_remote_function<protocol::echo_blob> echo_blob {endpoint}

#define require_blob_functions_le(limit, repetitions)                     \
  BOOST_REQUIRE_LE                                                        \
    ( fhg::util::testing::measure_average_time<std::chrono::microseconds> \
        ([&] { std::vector<char> const copy (get_blob()); }, repetitions) \
    , limit                                                               \
    );                                                                    \
  BOOST_REQUIRE_LE                                                        \
    ( fhg::util::testing::measure_average_time<std::chrono::microseconds> \
        ([&] { put_blob (blob); }, repetitions)                           \
    , limit                                                               \
    );                                                                    \
  BOOST_REQUIRE_LE                                                        \
    ( fhg::util::testing::measure_average_time<std::chrono::microseconds> \
        ([&] { std::vector<char> const copy (echo_blob (blob)); }, repetitions) \
    , limit                                                               \
    )

namespace data
{
  namespace
  {
    std::vector<std::size_t> powers_of_two (std::size_t from, std::size_t to)
    {
      std::vector<std::size_t> result;
      for (std::size_t exponent (from); exponent <= to; ++exponent)
      {
        result.emplace_back (std::size_t (1) << exponent);
      }
      return result;
    }
  }
}

BOOST_DATA_TEST_CASE ( below_1kb_shall_be_at_most_400_us
                     , data::powers_of_two (0, 9)
                     , size
                     )
{
  service_and_client_setup_with_blob_size (size);

  require_blob_functions_le (std::chrono::microseconds (400), 50);
}

BOOST_DATA_TEST_CASE ( below_128kb_shall_be_at_most_1_5_ms
                     , data::powers_of_two (10, 17)
                     , size
                     )
{
  service_and_client_setup_with_blob_size (size);

  require_blob_functions_le (std::chrono::microseconds (1500), 50);
}

BOOST_DATA_TEST_CASE ( above_1mb_throughput_shall_be_at_least_80_mibs
                     , data::powers_of_two (20, 30)
                     , size
                     )
{
  service_and_client_setup_with_blob_size (size);

  auto const limit ( std::chrono::duration_cast<std::chrono::milliseconds>
                       (std::chrono::duration<double> ((size >> 20) / (80.0)))
                   );

  require_blob_functions_le (limit, 5);
}

#define multiple_clients_with_one_server_setup( server_io_service_threads \
                                              , client_count              \
                                              , client_io_service_threads \
                                              )                           \
  fhg::util::scoped_boost_asio_io_service_with_threads server_io_service  \
    {server_io_service_threads};                                          \
  fhg::util::scoped_boost_asio_io_service_with_threads client_io_service  \
    {client_io_service_threads};                                          \
                                                                          \
  fhg::rpc::service_dispatcher service_dispatcher                         \
    {fhg::util::serialization::exception::serialization_functions()};     \
  fhg::rpc::service_handler<protocol::nop> nop_service                    \
    {service_dispatcher, []{}};                                           \
  fhg::rpc::service_tcp_provider const server                             \
    {server_io_service, service_dispatcher};                              \
                                                                          \
  std::string const endpoint_address                                      \
    ( fhg::util::connectable_to_address_string                            \
        (server.local_endpoint().address())                               \
    );                                                                    \
  unsigned short const endpoint_port (server.local_endpoint().port());    \
                                                                          \
  std::list<fhg::rpc::remote_tcp_endpoint> endpoints;                     \
  std::vector<fhg::rpc::remote_function<protocol::nop>> functions;        \
  for (std::size_t i (0); i < (client_count); ++i)                        \
  {                                                                       \
    endpoints.emplace_back                                                \
      (client_io_service, endpoint_address, endpoint_port);               \
    functions.emplace_back (endpoints.back());                            \
  }                                                                       \
  /* require semicolon without scope */ (void) 0

#define require_nop_calls_per_second_ge( limit                            \
                                       , invocations_per_client           \
                                       , repetitions                      \
                                       )                                  \
  do                                                                      \
  {                                                                       \
    auto const time                                                       \
      ( fhg::util::testing::measure_average_time<std::chrono::microseconds> \
          ( [&]                                                           \
            {                                                             \
              std::vector<std::future<void>> futures;                     \
              for (auto& function : functions)                            \
              {                                                           \
                for (std::size_t i (0); i < (invocations_per_client); ++i) \
                {                                                         \
                  futures.emplace_back (function());                      \
                }                                                         \
              }                                                           \
              for (auto& future : futures)                                \
              {                                                           \
                future.get();                                             \
              }                                                           \
            }                                                             \
          , repetitions                                                   \
          ).count()                                                       \
      );                                                                  \
                                                                          \
    BOOST_REQUIRE_GE                                                      \
      ( double (functions.size()) * double (invocations_per_client)       \
      / double (time) * std::micro::den                                   \
      , double (limit)                                                    \
      );                                                                  \
  } while (0)

namespace data
{
  template<std::size_t... Values>
    decltype (::boost::unit_test::data::make (std::vector<std::size_t>()))
      values()
  {
    return ::boost::unit_test::data::make (std::vector<std::size_t> ({Values...}));
  }
}

BOOST_DATA_TEST_CASE
  ( require_minimum_message_rates
  , ( data::values<1, 10, 20, 100, 200, 200, 200, 500>()
    ^ data::values<1, 10, 20, 20,  20,  1,   20,  20>()
    ^ data::values<1, 10, 20, 20,  20,  20,  1,   20>()
    )
  , client_count
  , server_thread_count
  , client_thread_count
  )
{
  multiple_clients_with_one_server_setup
    (server_thread_count, client_count, client_thread_count);

  auto const requirement (10000);
  require_nop_calls_per_second_ge (requirement, 10000 / client_count, 5);
}
