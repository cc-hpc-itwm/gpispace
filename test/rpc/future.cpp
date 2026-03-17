#include <gspc/rpc/future.hpp>
#include <gspc/rpc/remote_function.hpp>
#include <gspc/rpc/remote_tcp_endpoint.hpp>
#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>
#include <gspc/testing/printer/future.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/random/string.hpp>
#include <gspc/testing/require_exception.hpp>
#include <gspc/testing/require_type.hpp>

#include <boost/test/unit_test.hpp>


  namespace gspc::rpc
  {
    BOOST_AUTO_TEST_CASE (promise_can_only_be_satisfied_once_void)
    {
      ::boost::asio::io_service io_service;

      {
        promise<void> prom (io_service);

        prom.set_value();
        gspc::testing::require_exception
          ( [&]
            {
              prom.set_value();
            }
          , std::future_error
              (std::future_errc::promise_already_satisfied)
          );
      }

      {
        promise<void> prom (io_service);

        prom.set_exception (nullptr);
        gspc::testing::require_exception
          ( [&]
            {
              prom.set_value();
            }
          , std::future_error
              (std::future_errc::promise_already_satisfied)
          );
      }

      {
        promise<void> prom (io_service);

        prom.set_value();
        gspc::testing::require_exception
          ( [&]
            {
              prom.set_exception (nullptr);
            }
          , std::future_error
              (std::future_errc::promise_already_satisfied)
          );
      }

      {
        promise<void> prom (io_service);

        prom.set_exception (nullptr);
        gspc::testing::require_exception
          ( [&]
            {
              prom.set_exception (nullptr);
            }
          , std::future_error
              (std::future_errc::promise_already_satisfied)
          );
      }
    }

    BOOST_AUTO_TEST_CASE (future_can_only_be_retrieved_once_void)
    {
      ::boost::asio::io_service io_service;

      promise<void> prom (io_service);

      future<void> future (prom.get_future());

      gspc::testing::require_exception
        ( [&]
          {
            prom.get_future();
          }
        , std::future_error
            (std::future_errc::future_already_retrieved)
        );
    }

    BOOST_AUTO_TEST_CASE (future_can_be_set_and_get)
    {
      {
        ::boost::asio::io_service io_service;
        promise<std::string> prom (io_service);
        future<std::string> future (prom.get_future());

        std::string value (gspc::testing::random<std::string>{}());

        prom.set_value (value);

        GSPC_TESTING_REQUIRE_TYPE_EQUAL
          (std::string, decltype (future.get()));
        BOOST_REQUIRE_EQUAL (future.get(), value);
      }

      {
        ::boost::asio::io_service io_service;
        promise<std::string> prom (io_service);
        future<std::string> future (prom.get_future());

        //! \note Can call any constructor, emplace-style.
        unsigned char const count (gspc::testing::random<unsigned char>{}());
        char const what (gspc::testing::random<char>{}());

        prom.set_value (count, what);

        GSPC_TESTING_REQUIRE_TYPE_EQUAL
          (std::string, decltype (future.get()));
        BOOST_REQUIRE_EQUAL (future.get(), std::string (count, what));
      }

      {
        ::boost::asio::io_service io_service;
        promise<void> prom (io_service);
        future<void> future (prom.get_future());

        prom.set_value();

        GSPC_TESTING_REQUIRE_TYPE_EQUAL (void, decltype (future.get()));
        future.get();
      }

      {
        ::boost::asio::io_service io_service;
        promise<void> prom (io_service);
        future<void> future (prom.get_future());

        auto const exception_what (gspc::testing::random_string());
        prom.set_exception
          (std::make_exception_ptr (std::runtime_error (exception_what)));

        gspc::testing::require_exception
          ( [&]
            {
              future.get();
            }
          , std::runtime_error (exception_what)
          );
      }
    }

    BOOST_AUTO_TEST_CASE (future_can_be_set_and_threaded_get)
    {
      ::boost::asio::io_service io_service;
      promise<int> prom (io_service);
      future<int> future (prom.get_future());

      auto got ( std::async ( std::launch::async
                            , [&]
                              {
                                return future.get();
                              }
                            )
               );

      BOOST_REQUIRE_EQUAL ( got.wait_for (std::chrono::milliseconds (100))
                          , std::future_status::timeout
                          );

      int const value (gspc::testing::random<int>{}());

      prom.set_value (value);

      BOOST_REQUIRE_EQUAL (got.get(), value);
    }

    BOOST_AUTO_TEST_CASE (get_has_yielding_overload)
    {
      //! \note Example as to why we need this:
      {
        gspc::util::scoped_boost_asio_io_service_with_threads io_service_with_thread (1);
        ::boost::asio::io_service& io_service (io_service_with_thread);
        promise<int> prom (io_service);
        future<int> future (prom.get_future());

        int const value (gspc::testing::random<int>{}());

        std::promise<int> res;
        std::future<int> got (res.get_future());

        ::boost::asio::spawn ( io_service
                           , [&] (::boost::asio::yield_context)
                             {
                               res.set_value (future.get());
                             }
                           );
        ::boost::asio::spawn ( io_service
                           , [&] (::boost::asio::yield_context)
                             {
                               prom.set_value (value);
                             }
                           );

        BOOST_REQUIRE_EQUAL ( got.wait_for (std::chrono::milliseconds (100))
                            , std::future_status::timeout
                            );

        //! \note A second thread so that set_value can execute.
        static_cast<::boost::asio::io_service&> (io_service).run_one();

        BOOST_REQUIRE_EQUAL (got.get(), value);
      }

      {
        gspc::util::scoped_boost_asio_io_service_with_threads io_service_with_thread (1);
        ::boost::asio::io_service& io_service (io_service_with_thread);
        promise<int> prom (io_service);
        future<int> future (prom.get_future());

        int const value (gspc::testing::random<int>{}());

        std::promise<int> res;
        std::future<int> got (res.get_future());

        ::boost::asio::spawn ( io_service
                           , [&] (::boost::asio::yield_context yield)
                             {
                               res.set_value (future.get (yield));
                             }
                           );
        ::boost::asio::spawn ( io_service
                           , [&] (::boost::asio::yield_context)
                             {
                               prom.set_value (value);
                             }
                           );

        //! \note No second thread!

        BOOST_REQUIRE_EQUAL (got.get(), value);
      }
    }

    namespace protocol
    {
      FHG_RPC_FUNCTION_DESCRIPTION (prefix_sum, int (int));
    }

    BOOST_AUTO_TEST_CASE (future_enables_recursive_rpc_function_calls)
    {
      std::pair<std::string, unsigned short> address;

      gspc::util::scoped_boost_asio_io_service_with_threads io_service_server (1);
      gspc::util::scoped_boost_asio_io_service_with_threads io_service_client (1);

      service_dispatcher service_dispatcher;
      service_handler<protocol::prefix_sum> start_service
        ( service_dispatcher
        , [&] (::boost::asio::yield_context yield, int i)
          {
            // Blocking. Fine: Not server io_service but one dedicated
            // to just the callbacks: The client can't be blocked by
            // this handler itself.
            remote_tcp_endpoint client (io_service_client, address);
            remote_function<protocol::prefix_sum, future> prefix_sum (client);

            return i == 0 ? 0 : i + prefix_sum (i - 1).get (yield);
          }
        , yielding
        );
      service_tcp_provider const server {io_service_server, service_dispatcher};
      address = gspc::util::connectable_to_address_string (server.local_endpoint());

      remote_tcp_endpoint client (io_service_client, address);

      BOOST_REQUIRE_EQUAL
        ( sync_remote_function<protocol::prefix_sum> (client) (10)
        , 10 + 9 + 8 + 7 + 6 + 5 + 4 + 3 + 2 + 1 + 0
        );
    }

    BOOST_AUTO_TEST_CASE
      (future_enables_recursive_rpc_function_calls_single_io_service)
    {
      std::pair<std::string, unsigned short> address;

      gspc::util::scoped_boost_asio_io_service_with_threads io_service (2);

      service_dispatcher service_dispatcher;
      service_handler<protocol::prefix_sum> start_service
        ( service_dispatcher
        , [&] (::boost::asio::yield_context yield, int i)
          {
            remote_tcp_endpoint client (io_service, address);
            remote_function<protocol::prefix_sum, future> prefix_sum (client);

            // Blocking. Fine: yields.
            return i == 0 ? 0 : i + prefix_sum (i - 1).get (yield);
          }
        , yielding
        );
      service_tcp_provider const server {io_service, service_dispatcher};
      address = gspc::util::connectable_to_address_string (server.local_endpoint());

      remote_tcp_endpoint client (io_service, address);

      BOOST_REQUIRE_EQUAL
        ( sync_remote_function<protocol::prefix_sum> (client) (10)
        , 10 + 9 + 8 + 7 + 6 + 5 + 4 + 3 + 2 + 1 + 0
        );
    }

    BOOST_AUTO_TEST_CASE
      (future_enables_recursive_rpc_function_calls_single_io_service_sync_fun)
    {
      std::pair<std::string, unsigned short> address;

      gspc::util::scoped_boost_asio_io_service_with_threads io_service (2);

      service_dispatcher service_dispatcher;
      service_handler<protocol::prefix_sum> start_service
        ( service_dispatcher
        , [&] (::boost::asio::yield_context yield, int i)
          {
            remote_tcp_endpoint client (io_service, address);
            sync_remote_function<protocol::prefix_sum, future>
              prefix_sum (client);

            // Blocking. Fine: yields.
            return i == 0 ? 0 : i + prefix_sum (yield, i - 1);
          }
        , yielding
        );
      service_tcp_provider const server {io_service, service_dispatcher};
      address = gspc::util::connectable_to_address_string (server.local_endpoint());

      remote_tcp_endpoint client (io_service, address);

      BOOST_REQUIRE_EQUAL
        ( sync_remote_function<protocol::prefix_sum> (client) (10)
        , 10 + 9 + 8 + 7 + 6 + 5 + 4 + 3 + 2 + 1 + 0
        );
    }
  }
