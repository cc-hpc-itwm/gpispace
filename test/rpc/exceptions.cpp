#include <gspc/rpc/remote_tcp_endpoint.hpp>
#include <gspc/rpc/remote_function.hpp>
#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/tuple.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/serialization/list.hpp>
#include <boost/test/unit_test.hpp>

namespace protocol
{
  FHG_RPC_FUNCTION_DESCRIPTION (require_lt_0, void (int));
}

BOOST_AUTO_TEST_CASE (std_exceptions)
{
  gspc::rpc::service_dispatcher service_dispatcher;
  gspc::rpc::service_handler<protocol::require_lt_0> start_service
    ( service_dispatcher
    , [] (int i)
      {
        if (i >= 0)
        {
          throw std::invalid_argument (std::to_string (i) + " >= 0");
        }
      }
    );
  gspc::util::scoped_boost_asio_io_service_with_threads io_service_server (1);
  gspc::rpc::service_tcp_provider const server
    {io_service_server, service_dispatcher};

  gspc::util::scoped_boost_asio_io_service_with_threads io_service_client (1);
  gspc::rpc::remote_tcp_endpoint endpoint
    ( io_service_client
    , gspc::util::connectable_to_address_string (server.local_endpoint())
    );
  gspc::rpc::sync_remote_function<protocol::require_lt_0> require_lt_0 (endpoint);

  require_lt_0 (-1);
  gspc::testing::require_exception
    ( [&] { require_lt_0 (1); }
    , std::invalid_argument ("1 >= 0")
    );
}

namespace
{
  struct user_defined_exception
  {
    int dummy;
    user_defined_exception (int d)
      : dummy (d)
    {}
    //! \note Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=62154
    virtual ~user_defined_exception() = default;
    user_defined_exception (user_defined_exception const&) = default;

    std::string what() const
    {
      return "dummy = " + std::to_string (dummy);
    }

    static std::optional<std::string> from_exception_ptr
      (std::exception_ptr ex_ptr)
    {
      try
      {
        std::rethrow_exception (ex_ptr);
      }
      catch (user_defined_exception const& ex)
      {
        return std::to_string (ex.dummy);
      }
      catch (...)
      {
        return {};
      }
    }
    [[noreturn]] static void throw_with_nested (std::string serialized)
    {
      std::throw_with_nested (user_defined_exception (std::stoi (serialized)));
    }
    static std::exception_ptr to_exception_ptr (std::string serialized)
    {
      return std::make_exception_ptr
        (user_defined_exception (std::stoi (serialized)));
    }
  };
  struct user_defined_std_runtime_error : std::runtime_error
  {
    int dummy;
    user_defined_std_runtime_error (int d)
      : std::runtime_error ("dummy = " + std::to_string (d))
      , dummy (d)
    {}

    static std::optional<std::string> from_exception_ptr
      (std::exception_ptr ex_ptr)
    {
      try
      {
        std::rethrow_exception (ex_ptr);
      }
      catch (user_defined_std_runtime_error const& ex)
      {
        return std::to_string (ex.dummy);
      }
      catch (...)
      {
        return {};
      }
    }
    [[noreturn]] static void throw_with_nested (std::string serialized)
    {
      std::throw_with_nested (user_defined_exception (std::stoi (serialized)));
    }
    static std::exception_ptr to_exception_ptr (std::string serialized)
    {
      return std::make_exception_ptr
        (user_defined_std_runtime_error (std::stoi (serialized)));
    }
  };
}

namespace protocol
{
  FHG_RPC_FUNCTION_DESCRIPTION (throw_exception, void (bool, int));
}

BOOST_AUTO_TEST_CASE (user_defined_exceptions)
{
  gspc::rpc::service_dispatcher service_dispatcher
    ( gspc::util::serialization::exception::serialization_functions
      { { "ude"
        , { &user_defined_exception::from_exception_ptr
          , &user_defined_exception::to_exception_ptr
          , &user_defined_exception::throw_with_nested
          }
        }
      , { "udsrte"
        , { &user_defined_std_runtime_error::from_exception_ptr
          , &user_defined_std_runtime_error::to_exception_ptr
          , &user_defined_std_runtime_error::throw_with_nested
          }
        }
      }
    );
  gspc::rpc::service_handler<protocol::throw_exception> start_service
    ( service_dispatcher
    , [] (bool std_runtime_error_based, int dummy)
      {
        if (std_runtime_error_based)
        {
          throw user_defined_std_runtime_error (dummy);
        }
        else
        {
          throw user_defined_exception (dummy);
        }
      }
    );
  gspc::util::scoped_boost_asio_io_service_with_threads io_service_server (1);
  gspc::rpc::service_tcp_provider const server
    {io_service_server, service_dispatcher};

  gspc::util::scoped_boost_asio_io_service_with_threads io_service_client (1);
    gspc::rpc::remote_tcp_endpoint endpoint
    ( io_service_client
    , gspc::util::connectable_to_address_string (server.local_endpoint())
    , gspc::util::serialization::exception::serialization_functions
      { { "ude", { &user_defined_exception::from_exception_ptr
                 , &user_defined_exception::to_exception_ptr
                 , &user_defined_exception::throw_with_nested
                 }
        }
      , { "udsrte", { &user_defined_std_runtime_error::from_exception_ptr
                    , &user_defined_std_runtime_error::to_exception_ptr
                    , &user_defined_std_runtime_error::throw_with_nested
                    }
        }
      }
    );
  gspc::rpc::sync_remote_function<protocol::throw_exception> throw_exception
    (endpoint);

  {
    int const s (gspc::testing::random<int>{}());
    gspc::testing::require_exception
      ( [&] { throw_exception (false, s); }
      , user_defined_exception (s)
      );
  }
  {
    int const s (gspc::testing::random<int>{}());
    gspc::testing::require_exception
      ( [&] { throw_exception (true, s); }
      , user_defined_std_runtime_error (s)
      );
  }
}

namespace protocol
{
  FHG_RPC_FUNCTION_DESCRIPTION (throw_udsre_exception, void (int));
}

BOOST_AUTO_TEST_CASE (user_defined_std_exceptions_are_downcast_automatically_server)
{
  gspc::rpc::service_dispatcher service_dispatcher;
  gspc::rpc::service_handler<protocol::throw_udsre_exception> start_service
    ( service_dispatcher
    , [] (int dummy)
      {
        throw user_defined_std_runtime_error (dummy);
      }
    );
  gspc::util::scoped_boost_asio_io_service_with_threads io_service_server (1);
  gspc::rpc::service_tcp_provider const server
    {io_service_server, service_dispatcher};

  gspc::util::scoped_boost_asio_io_service_with_threads io_service_client (1);
  gspc::rpc::remote_tcp_endpoint endpoint
    ( io_service_client
    , gspc::util::connectable_to_address_string (server.local_endpoint())
    );
  gspc::rpc::sync_remote_function<protocol::throw_udsre_exception> throw_exception
    (endpoint);

  {
    int const s (gspc::testing::random<int>{}());
    gspc::testing::require_exception
      ( [&] { throw_exception (s); }
      , std::runtime_error ("dummy = " + std::to_string (s))
      );
  }
}

BOOST_AUTO_TEST_CASE (user_defined_std_exceptions_are_downcast_automatically_client)
{
  gspc::rpc::service_dispatcher service_dispatcher
    ( gspc::util::serialization::exception::serialization_functions
      { { "udsrte"
        , { &user_defined_std_runtime_error::from_exception_ptr
          , &user_defined_std_runtime_error::to_exception_ptr
          , &user_defined_std_runtime_error::throw_with_nested
          }
        }
      }
    );
  gspc::rpc::service_handler<protocol::throw_udsre_exception> start_service
    ( service_dispatcher
    , [] (int dummy)
      {
        throw user_defined_std_runtime_error (dummy);
      }
    );
  gspc::util::scoped_boost_asio_io_service_with_threads io_service_server (1);
  gspc::rpc::service_tcp_provider const server
    {io_service_server, service_dispatcher};

  gspc::util::scoped_boost_asio_io_service_with_threads io_service_client (1);
  gspc::rpc::remote_tcp_endpoint endpoint
    ( io_service_client
    , gspc::util::connectable_to_address_string (server.local_endpoint())
    );
  gspc::rpc::sync_remote_function<protocol::throw_udsre_exception> throw_exception
    (endpoint);

  {
    int const s (gspc::testing::random<int>{}());
    gspc::testing::require_exception
      ( [&] { throw_exception (s); }
      , std::runtime_error ("dummy = " + std::to_string (s))
      );
  }
}

namespace protocol
{
  FHG_RPC_FUNCTION_DESCRIPTION (throw_nested_exception, void (bool, int));
}

BOOST_AUTO_TEST_CASE (nested_exceptions)
{
  gspc::rpc::service_dispatcher service_dispatcher
    ( gspc::util::serialization::exception::serialization_functions
      { { "ude"
        , { &user_defined_exception::from_exception_ptr
          , &user_defined_exception::to_exception_ptr
          , &user_defined_exception::throw_with_nested
          }
        }
      }
    );
  gspc::rpc::service_handler<protocol::throw_nested_exception> start_service
    ( service_dispatcher
    , [] (bool std_logic_error, int dummy)
      {
        try
        {
          if (std_logic_error)
          {
            throw std::invalid_argument (std::to_string (dummy));
          }
          else
          {
            throw user_defined_exception (dummy);
          }
        }
        catch (...)
        {
          std::throw_with_nested (std::runtime_error ("failing succeeded"));
        }
      }
    );
  gspc::util::scoped_boost_asio_io_service_with_threads io_service_server (1);
  gspc::rpc::service_tcp_provider const server
    {io_service_server, service_dispatcher};

  gspc::util::scoped_boost_asio_io_service_with_threads io_service_client (1);
  gspc::rpc::remote_tcp_endpoint endpoint
    ( io_service_client
    , gspc::util::connectable_to_address_string (server.local_endpoint())
    , gspc::util::serialization::exception::serialization_functions
      { { "ude", { &user_defined_exception::from_exception_ptr
                 , &user_defined_exception::to_exception_ptr
                 , &user_defined_exception::throw_with_nested
                 }
        }
      }
    );
  gspc::rpc::sync_remote_function<protocol::throw_nested_exception> throw_exception
    (endpoint);

  {
    int const s (gspc::testing::random<int>{}());
    try
    {
      throw_exception (false, s);
    }
    catch (std::runtime_error const& ex)
    {
      BOOST_REQUIRE_EQUAL (ex.what(), "failing succeeded");
      try
      {
        std::rethrow_if_nested (ex);
        BOOST_FAIL ("requires top level to be nesting");
      }
      catch (user_defined_exception const& ude)
      {
        BOOST_REQUIRE_EQUAL (ude.dummy, s);
        BOOST_REQUIRE_EQUAL (ude.what(), "dummy = " + std::to_string (s));
      }
    }
  }
  {
    int const s (gspc::testing::random<int>{}());
    try
    {
      throw_exception (true, s);
    }
    catch (std::runtime_error const& ex)
    {
      BOOST_REQUIRE_EQUAL (ex.what(), "failing succeeded");
      try
      {
        std::rethrow_if_nested (ex);
        BOOST_FAIL ("requires top level to be nesting");
      }
      catch (std::invalid_argument const& nested)
      {
        BOOST_REQUIRE_EQUAL (nested.what(), std::to_string (s));
      }
    }
  }
}
