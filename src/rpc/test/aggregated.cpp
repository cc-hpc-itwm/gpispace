// bernd.loerwald@itwm.fraunhofer.de
#define BOOST_TEST_MODULE rpc

#include <rpc/test/helpers.hpp>

#include <rpc/client.hpp>

#include <fhg/util/boost/asio/ip/address.hpp>
#include <fhg/util/boost/test/printer/tuple.hpp>
#include <fhg/util/boost/test/require_exception.hpp>

#include <boost/serialization/list.hpp>
#include <boost/test/unit_test.hpp>

//! \todo cross-process test cases?

namespace
{
  std::list<fhg::rpc::endpoints_type> unary_split
    (fhg::rpc::endpoints_type endpoints)
  {
    if (endpoints.empty())
    {
      return {};
    }
    else if (endpoints.size() == 1)
    {
      return {endpoints};
    }
    else
    {
      return { {endpoints.front()}
             , {endpoints.begin() + 1, endpoints.end()}
             };
    }
  }
  std::list<fhg::rpc::endpoints_type> binary_split
    (fhg::rpc::endpoints_type endpoints)
  {
    if (endpoints.empty())
    {
      return {};
    }
    else if (endpoints.size() == 1)
    {
      return {endpoints};
    }
    else
    {
      return { {endpoints.begin(), endpoints.begin() + endpoints.size() / 2}
             , {endpoints.begin() + endpoints.size() / 2, endpoints.end()}
             };
    }
  }
  fhg::rpc::endpoint_type front (fhg::rpc::endpoints_type endpoints)
  {
    return endpoints.front();
  }
  template<typename Servers>
    fhg::rpc::endpoints_type endpoints (Servers const& servers)
  {
    fhg::rpc::endpoints_type ret;
    for (auto const& server : servers)
    {
      ret.push_back (server.server.acceptor.local_endpoint());
    }
    return ret;
  }
}

BOOST_AUTO_TEST_CASE (aggregated_int_ping)
{
  struct server
  {
    fhg::rpc::service_dispatcher service_dispatcher
      {fhg::rpc::exception::serialization_functions()};
    fhg::rpc::aggregated_service_handler<int (int)> start_service
      { service_dispatcher
      , "ping"
      , [] (int i) { return i + 1; }
      , std::bind (&server::is_endpoint, this, std::placeholders::_1)
      , &binary_split
      , &front
      , std::bind (&server::io_service, this)
      };
    server_for_dispatcher server {service_dispatcher};
    bool is_endpoint (fhg::rpc::endpoint_type endpoint) const
    {
      return endpoint == server.acceptor.local_endpoint();
    }
    io_service_with_work_thread_and_stop_on_scope_exit _io_service;
    boost::asio::io_service& io_service()
    {
      return _io_service.service;
    }
  };

  std::list<server> servers (2);
  server& entry (servers.front());

  io_service_with_work_thread_and_stop_on_scope_exit io_service_client;
  fhg::rpc::remote_endpoint endpoint
    ( io_service_client.service
    , fhg::util::connectable_to_address_string
        (entry.server.acceptor.local_endpoint().address())
    , entry.server.acceptor.local_endpoint().port()
    , fhg::rpc::exception::serialization_functions()
    );
  fhg::rpc::sync_aggregated_remote_function<int (int)> ping (endpoint, "ping");

  int const s (rand());
  for ( std::pair<fhg::rpc::endpoint_type, int> result
      : ping (endpoints (servers), s)
      )
  {
    BOOST_REQUIRE_EQUAL (result.second, s + 1);
  }
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

BOOST_AUTO_TEST_CASE (aggregated_user_defined_type_echo)
{
  struct server
  {
    fhg::rpc::service_dispatcher service_dispatcher
      {fhg::rpc::exception::serialization_functions()};
    fhg::rpc::aggregated_service_handler
      <user_defined_type (user_defined_type)> service
      { service_dispatcher
      , "echo"
      , [] (user_defined_type x) { return x; }
      , std::bind (&server::is_endpoint, this, std::placeholders::_1)
      , &binary_split
      , &front
      , std::bind (&server::io_service, this)
      };
    server_for_dispatcher server {service_dispatcher};
    bool is_endpoint (fhg::rpc::endpoint_type endpoint) const
    {
      return endpoint == server.acceptor.local_endpoint();
    }
    io_service_with_work_thread_and_stop_on_scope_exit _io_service;
    boost::asio::io_service& io_service()
    {
      return _io_service.service;
    }
  };

  std::list<server> servers (2);
  server& entry (servers.front());

  io_service_with_work_thread_and_stop_on_scope_exit io_service_client;
  fhg::rpc::remote_endpoint endpoint
    ( io_service_client.service
    , fhg::util::connectable_to_address_string
        (entry.server.acceptor.local_endpoint().address())
    , entry.server.acceptor.local_endpoint().port()
    , fhg::rpc::exception::serialization_functions()
    );
  fhg::rpc::sync_aggregated_remote_function
    <user_defined_type (user_defined_type)> echo (endpoint, "echo");

  user_defined_type const udt {"baz", {"brunz", "buu"}};
  for ( std::pair<fhg::rpc::endpoint_type, user_defined_type> result
      : echo (endpoints (servers), udt)
      )
  {
    BOOST_REQUIRE_EQUAL (result.second, udt);
  }
}

BOOST_AUTO_TEST_CASE (first_endpoint_not_in_target_endpoints)
{
  struct server
  {
    fhg::rpc::service_dispatcher service_dispatcher
      {fhg::rpc::exception::serialization_functions()};
    fhg::rpc::aggregated_service_handler<int (int)> start_service
      { service_dispatcher
      , "ping"
      , [] (int i) { return i + 1; }
      , std::bind (&server::is_endpoint, this, std::placeholders::_1)
      , &binary_split
      , &front
      , std::bind (&server::io_service, this)
      };
    server_for_dispatcher server {service_dispatcher};
    bool is_endpoint (fhg::rpc::endpoint_type endpoint) const
    {
      return endpoint == server.acceptor.local_endpoint();
    }
    io_service_with_work_thread_and_stop_on_scope_exit _io_service;
    boost::asio::io_service& io_service()
    {
      return _io_service.service;
    }
  };

  std::list<server> servers (2);
  server entry;

  io_service_with_work_thread_and_stop_on_scope_exit io_service_client;
  fhg::rpc::remote_endpoint endpoint
    ( io_service_client.service
    , fhg::util::connectable_to_address_string
        (entry.server.acceptor.local_endpoint().address())
    , entry.server.acceptor.local_endpoint().port()
    , fhg::rpc::exception::serialization_functions()
    );
  fhg::rpc::sync_aggregated_remote_function<int (int)> ping (endpoint, "ping");

  int const s (rand());
  for ( std::pair<fhg::rpc::endpoint_type, int> result
      : ping (endpoints (servers), s)
      )
  {
    BOOST_REQUIRE_EQUAL (result.second, s + 1);
  }
}

BOOST_AUTO_TEST_CASE (split_can_do_unary_tree_as_well)
{
  struct server
  {
    fhg::rpc::service_dispatcher service_dispatcher
      {fhg::rpc::exception::serialization_functions()};
    fhg::rpc::aggregated_service_handler<int (int)> start_service
      { service_dispatcher
      , "ping"
      , [] (int i) { return i + 1; }
      , std::bind (&server::is_endpoint, this, std::placeholders::_1)
      , &unary_split
      , &front
      , std::bind (&server::io_service, this)
      };
    server_for_dispatcher server {service_dispatcher};
    bool is_endpoint (fhg::rpc::endpoint_type endpoint) const
    {
      return endpoint == server.acceptor.local_endpoint();
    }
    io_service_with_work_thread_and_stop_on_scope_exit _io_service;
    boost::asio::io_service& io_service()
    {
      return _io_service.service;
    }
  };

  std::list<server> servers (10);
  server& entry (servers.front());

  io_service_with_work_thread_and_stop_on_scope_exit io_service_client;
  fhg::rpc::remote_endpoint endpoint
    ( io_service_client.service
    , fhg::util::connectable_to_address_string
        (entry.server.acceptor.local_endpoint().address())
    , entry.server.acceptor.local_endpoint().port()
    , fhg::rpc::exception::serialization_functions()
    );
  fhg::rpc::sync_aggregated_remote_function<int (int)> ping (endpoint, "ping");

  int const s (rand());
  for ( std::pair<fhg::rpc::endpoint_type, int> result
      : ping (endpoints (servers), s)
      )
  {
    BOOST_REQUIRE_EQUAL (result.second, s + 1);
  }
}

namespace
{
  struct user_defined_exception
  {
    std::string dummy;
    user_defined_exception (std::string d)
      : dummy (d)
    {}
    //! \note Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=62154
    virtual ~user_defined_exception() = default;
    std::string what() const
    {
      return dummy;
    }

    static boost::optional<std::string> from_exception_ptr
      (std::exception_ptr ex_ptr)
    {
      try
      {
        std::rethrow_exception (ex_ptr);
      }
      catch (user_defined_exception const& ex)
      {
        return ex.dummy;
      }
      catch (...)
      {
        return boost::none;
      }
    }
    static void throw_with_nested (std::string serialized)
    {
      std::throw_with_nested (user_defined_exception (serialized));
    }
    static std::exception_ptr to_exception_ptr (std::string serialized)
    {
      return std::make_exception_ptr (user_defined_exception (serialized));
    }
  };
}

BOOST_AUTO_TEST_CASE (exceptions_are_aggregated_together_with_results)
{
  struct server
  {
    fhg::rpc::service_dispatcher service_dispatcher
      {{{ "ude"
        , { &user_defined_exception::from_exception_ptr
          , &user_defined_exception::to_exception_ptr
          , &user_defined_exception::throw_with_nested
          }
        }
       }
      };
    fhg::rpc::aggregated_service_handler
      <user_defined_type (user_defined_type)> service
      { service_dispatcher
      , "echo"
      , [] (user_defined_type x) { return x; }
      , std::bind (&server::is_endpoint, this, std::placeholders::_1)
      , &binary_split
      , &front
      , std::bind (&server::io_service, this)
      };
    server_for_dispatcher server {service_dispatcher};
    bool is_endpoint (fhg::rpc::endpoint_type endpoint) const
    {
      return endpoint == server.acceptor.local_endpoint();
    }
    io_service_with_work_thread_and_stop_on_scope_exit _io_service;
    boost::asio::io_service& io_service()
    {
      return _io_service.service;
    }
  };
  struct broken_server
  {
    fhg::rpc::service_dispatcher service_dispatcher
      {{{ "ude"
        , { &user_defined_exception::from_exception_ptr
          , &user_defined_exception::to_exception_ptr
          , &user_defined_exception::throw_with_nested
          }
        }
       }
      };
    fhg::rpc::aggregated_service_handler
      <user_defined_type (user_defined_type)> service
      { service_dispatcher
      , "echo"
      , [] (user_defined_type x) -> user_defined_type
        {
          throw user_defined_exception (x.foo);
        }
      , std::bind (&broken_server::is_endpoint, this, std::placeholders::_1)
      , &binary_split
      , &front
      , std::bind (&broken_server::io_service, this)
      };
    server_for_dispatcher server {service_dispatcher};
    bool is_endpoint (fhg::rpc::endpoint_type endpoint) const
    {
      return endpoint == server.acceptor.local_endpoint();
    }
    io_service_with_work_thread_and_stop_on_scope_exit _io_service;
    boost::asio::io_service& io_service()
    {
      return _io_service.service;
    }
  };

  std::list<server> servers (2);
  broken_server broken_server;
  server& entry (servers.front());
  fhg::rpc::endpoints_type eps (endpoints (servers));
  fhg::rpc::endpoint_type broken_server_endpoint
    (broken_server.server.acceptor.local_endpoint());
  eps.push_back (broken_server_endpoint);

  io_service_with_work_thread_and_stop_on_scope_exit io_service_client;
  fhg::rpc::remote_endpoint endpoint
    ( io_service_client.service
    , fhg::util::connectable_to_address_string
        (entry.server.acceptor.local_endpoint().address())
    , entry.server.acceptor.local_endpoint().port()
    , { { "ude", { &user_defined_exception::from_exception_ptr
                 , &user_defined_exception::to_exception_ptr
                 , &user_defined_exception::throw_with_nested
                 }
        }
      }
    );
  fhg::rpc::sync_aggregated_remote_function
    <user_defined_type (user_defined_type)> echo (endpoint, "echo");

  user_defined_type const udt {"baz", {"brunz", "buu"}};
  try
  {
    echo (eps, udt);
    BOOST_FAIL ("expected exception");
  }
  catch (fhg::rpc::aggregated_exception<user_defined_type> const& ex)
  {
    BOOST_REQUIRE_EQUAL (ex.what(), std::string ("1 exceptions"));
    BOOST_REQUIRE_EQUAL (ex.succeeded.size(), 2);
    for ( std::pair<fhg::rpc::endpoint_type, user_defined_type> result
        : ex.succeeded
        )
    {
      BOOST_REQUIRE_EQUAL (result.second, udt);
    }

    BOOST_REQUIRE_EQUAL (ex.failed.size(), 1);
    BOOST_REQUIRE_EQUAL (ex.failed.begin()->first, broken_server_endpoint);
    try
    {
      std::rethrow_exception (ex.failed.begin()->second);
      BOOST_FAIL ("expected exception");
    }
    catch (user_defined_exception const& ex)
    {
      BOOST_REQUIRE_EQUAL (ex.what(), udt.foo);
    }
  }
}
