// bernd.loerwald@itwm.fraunhofer.de
#define BOOST_TEST_MODULE rpc

#include <rpc/test/helpers.hpp>

#include <rpc/client.hpp>

#include <fhg/util/boost/asio/ip/address.hpp>
#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>
#include <fhg/util/boost/test/printer/tuple.hpp>
#include <fhg/util/boost/test/require_exception.hpp>

#include <boost/serialization/list.hpp>
#include <boost/test/unit_test.hpp>

//! \todo cross-process test cases?

FHG_BOOST_TEST_LOG_VALUE_PRINTER (std::future_status, os, status)
{
  switch (status)
  {
  case std::future_status::ready: os << "ready"; break;
  case std::future_status::timeout: os << "timeout"; break;
  case std::future_status::deferred: os << "deferred"; break;
  }
}

BOOST_AUTO_TEST_CASE (int_ping)
{
  fhg::rpc::service_dispatcher service_dispatcher
    {fhg::rpc::exception::serialization_functions()};
  fhg::rpc::service_handler<int (int)> start_service
    ( service_dispatcher
    , "ping"
    , [] (int i) { return i + 1; }
    );
  server_for_dispatcher server (service_dispatcher);

  io_service_with_work_thread_and_stop_on_scope_exit io_service_client;
  fhg::rpc::remote_endpoint endpoint
    ( io_service_client.service
    , fhg::util::connectable_to_address_string
        (server.acceptor.local_endpoint().address())
    , server.acceptor.local_endpoint().port()
    , fhg::rpc::exception::serialization_functions()
    );
  fhg::rpc::remote_function<int (int)> ping (endpoint, "ping");

  int const s (rand());
  std::future<int> pong (ping (s));

  BOOST_REQUIRE_EQUAL ( pong.wait_for (std::chrono::seconds (1))
                      , std::future_status::ready
                      );

  BOOST_REQUIRE_EQUAL (pong.get(), s + 1);
}

BOOST_AUTO_TEST_CASE (int_ping_sync)
{
  fhg::rpc::service_dispatcher service_dispatcher
    {fhg::rpc::exception::serialization_functions()};
  fhg::rpc::service_handler<int (int)> start_service
    ( service_dispatcher
    , "ping"
    , [] (int i) { return i + 1; }
    );
  server_for_dispatcher server (service_dispatcher);

  io_service_with_work_thread_and_stop_on_scope_exit io_service_client;
  fhg::rpc::remote_endpoint endpoint
    ( io_service_client.service
    , fhg::util::connectable_to_address_string
        (server.acceptor.local_endpoint().address())
    , server.acceptor.local_endpoint().port()
    , fhg::rpc::exception::serialization_functions()
    );
  fhg::rpc::sync_remote_function<int (int)> ping (endpoint, "ping");

  int const s (rand());
  BOOST_REQUIRE_EQUAL (ping (s), s + 1);
}

namespace
{
  struct user_defined_type
  {
    std::string foo;
    std::list<std::string> bar;
    bool operator== (user_defined_type const& rhs) const
    {
      return std::tie (foo, bar) == std::tie (rhs.foo, rhs.bar);
    }
  };
}
FHG_BOOST_TEST_LOG_VALUE_PRINTER (user_defined_type, os, udt)
{
  os << udt.foo << ", ";
  FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (udt.bar);
}
namespace boost
{
  namespace serialization
  {
    template<typename Archive>
      void serialize (Archive& ar, user_defined_type& udt, const unsigned int)
    {
      ar & udt.foo;
      ar & udt.bar;
    }
  }
}

BOOST_AUTO_TEST_CASE (user_defined_type_echo)
{
  fhg::rpc::service_dispatcher service_dispatcher
    {fhg::rpc::exception::serialization_functions()};
  fhg::rpc::service_handler<user_defined_type (user_defined_type)> start_service
    ( service_dispatcher
    , "echo"
    , [] (user_defined_type x) { return x; }
    );
  server_for_dispatcher server (service_dispatcher);

  io_service_with_work_thread_and_stop_on_scope_exit io_service_client;
  fhg::rpc::remote_endpoint endpoint
    ( io_service_client.service
    , fhg::util::connectable_to_address_string
        (server.acceptor.local_endpoint().address())
    , server.acceptor.local_endpoint().port()
    , fhg::rpc::exception::serialization_functions()
    );
  fhg::rpc::sync_remote_function<user_defined_type (user_defined_type)> echo
    (endpoint, "echo");


  user_defined_type const udt {"baz", {"brunz", "buu"}};
  BOOST_REQUIRE_EQUAL (echo (udt), udt);
}
