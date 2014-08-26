// bernd.loerwald@itwm.fraunhofer.de
#define BOOST_TEST_MODULE rpc

#include <rpc/test/helpers.hpp>

#include <rpc/client.hpp>

#include <fhg/util/boost/test/printer/tuple.hpp>
#include <fhg/util/boost/test/require_exception.hpp>

#include <boost/serialization/list.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (std_exceptions)
{
  fhg::rpc::service_dispatcher service_dispatcher
    {fhg::rpc::exception::serialization_functions()};
  fhg::rpc::service_handler start_service
    ( service_dispatcher
    , "require_lt_0"
    , fhg::rpc::thunk<void (int)>
      ([] (int i)
      {
        if (i >= 0)
        {
          throw std::invalid_argument (std::to_string (i) + " >= 0");
        }
      }
      )
    );
  server_for_dispatcher server (service_dispatcher);

  io_service_with_work_thread_and_stop_on_scope_exit io_service_client;
  fhg::rpc::remote_endpoint endpoint
    ( io_service_client.service
    , server.acceptor.local_endpoint().address().to_string()
    , server.acceptor.local_endpoint().port()
    , fhg::rpc::exception::deserialization_functions()
    );
  fhg::rpc::sync_remote_function<void (int)> require_lt_0
    (endpoint, "require_lt_0");

  require_lt_0 (-1);
  fhg::util::boost::test::require_exception<std::invalid_argument>
    (std::bind (require_lt_0, 1), "1 >= 0");
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
    std::string what() const
    {
      return "dummy = " + std::to_string (dummy);
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
        return std::to_string (ex.dummy);
      }
      catch (...)
      {
        return boost::none;
      }
    }
    static void throw_with_nested (std::string serialized)
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

    static boost::optional<std::string> from_exception_ptr
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
        return boost::none;
      }
    }
    static void throw_with_nested (std::string serialized)
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

BOOST_AUTO_TEST_CASE (user_defined_exceptions)
{
  fhg::rpc::service_dispatcher service_dispatcher
    ( { { "ude", &user_defined_exception::from_exception_ptr }
      , { "udsrte", &user_defined_std_runtime_error::from_exception_ptr }
      }
    );
  fhg::rpc::service_handler start_service
    ( service_dispatcher
    , "throw_exception"
    , fhg::rpc::thunk<void (bool, int)>
      ([] (bool std_runtime_error_based, int dummy)
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
      )
    );
  server_for_dispatcher server (service_dispatcher);

  io_service_with_work_thread_and_stop_on_scope_exit io_service_client;
  fhg::rpc::remote_endpoint endpoint
    ( io_service_client.service
    , server.acceptor.local_endpoint().address().to_string()
    , server.acceptor.local_endpoint().port()
    , { { "ude", { &user_defined_exception::to_exception_ptr
                 , &user_defined_exception::throw_with_nested
                 }
        }
      , { "udsrte", { &user_defined_std_runtime_error::to_exception_ptr
                    , &user_defined_std_runtime_error::throw_with_nested
                    }
        }
      }
    );
  fhg::rpc::sync_remote_function<void (bool, int)> throw_exception
    (endpoint, "throw_exception");

  {
    int const s (rand());
    fhg::util::boost::test::require_exception<user_defined_exception>
      (std::bind (throw_exception, false, s), "dummy = " + std::to_string (s));
  }
  {
    int const s (rand());
    fhg::util::boost::test::require_exception<user_defined_std_runtime_error>
      (std::bind (throw_exception, true, s), "dummy = " + std::to_string (s));
  }
}

BOOST_AUTO_TEST_CASE (user_defined_std_exceptions_are_downcast_automatically_server)
{
  fhg::rpc::service_dispatcher service_dispatcher
    {fhg::rpc::exception::serialization_functions()};
  fhg::rpc::service_handler start_service
    ( service_dispatcher
    , "throw_exception"
    , fhg::rpc::thunk<void (int)>
      ([] (int dummy)
      {
        throw user_defined_std_runtime_error (dummy);
      }
      )
    );
  server_for_dispatcher server (service_dispatcher);

  io_service_with_work_thread_and_stop_on_scope_exit io_service_client;
  fhg::rpc::remote_endpoint endpoint
    ( io_service_client.service
    , server.acceptor.local_endpoint().address().to_string()
    , server.acceptor.local_endpoint().port()
    , fhg::rpc::exception::deserialization_functions()
    );
  fhg::rpc::sync_remote_function<void (int)> throw_exception
    (endpoint, "throw_exception");

  {
    int const s (rand());
    fhg::util::boost::test::require_exception<std::runtime_error>
      (std::bind (throw_exception, s), "dummy = " + std::to_string (s));
  }
}

BOOST_AUTO_TEST_CASE (user_defined_std_exceptions_are_downcast_automatically_client)
{
  fhg::rpc::service_dispatcher service_dispatcher
    ({{"udsrte", &user_defined_std_runtime_error::from_exception_ptr}});
  fhg::rpc::service_handler start_service
    ( service_dispatcher
    , "throw_exception"
    , fhg::rpc::thunk<void (int)>
      ([] (int dummy)
      {
        throw user_defined_std_runtime_error (dummy);
      }
      )
    );
  server_for_dispatcher server (service_dispatcher);

  io_service_with_work_thread_and_stop_on_scope_exit io_service_client;
  fhg::rpc::remote_endpoint endpoint
    ( io_service_client.service
    , server.acceptor.local_endpoint().address().to_string()
    , server.acceptor.local_endpoint().port()
    , fhg::rpc::exception::deserialization_functions()
    );
  fhg::rpc::sync_remote_function<void (int)> throw_exception
    (endpoint, "throw_exception");

  {
    int const s (rand());
    fhg::util::boost::test::require_exception<std::runtime_error>
      (std::bind (throw_exception, s), "dummy = " + std::to_string (s));
  }
}

BOOST_AUTO_TEST_CASE (nested_exceptions)
{
  fhg::rpc::service_dispatcher service_dispatcher
    ({{"ude", &user_defined_exception::from_exception_ptr}});
  fhg::rpc::service_handler start_service
    ( service_dispatcher
    , "throw_exception"
    , fhg::rpc::thunk<void (bool, int)>
      ([] (bool std_logic_error, int dummy)
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
      )
    );
  server_for_dispatcher server (service_dispatcher);

  io_service_with_work_thread_and_stop_on_scope_exit io_service_client;
  fhg::rpc::remote_endpoint endpoint
    ( io_service_client.service
    , server.acceptor.local_endpoint().address().to_string()
    , server.acceptor.local_endpoint().port()
    , { { "ude", { &user_defined_exception::to_exception_ptr
                 , &user_defined_exception::throw_with_nested
                 }
        }
      }
    );
  fhg::rpc::sync_remote_function<void (bool, int)> throw_exception
    (endpoint, "throw_exception");

  {
    int const s (rand());
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
    int const s (rand());
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
