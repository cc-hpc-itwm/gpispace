#include <gspc/util/callable_signature.hpp>

#include <gspc/testing/require_type.hpp>

#include <boost/test/unit_test.hpp>


  namespace gspc::util
  {
    //! \note not in anonymous namespace to avoid clang not emitting
    //! it (-Wunneeded-member-function)
    struct fun
    {
      int operator() (fun const&) const;
      void member_function (std::string&&);
      static void static_function (std::string&&);
    };

    BOOST_AUTO_TEST_CASE (normal_signatures)
    {
      GSPC_TESTING_CHECK_TYPE_EQUAL
        (callable_signature<void()>, void());
      GSPC_TESTING_CHECK_TYPE_EQUAL
        (callable_signature<void (void)>, void());
      GSPC_TESTING_CHECK_TYPE_EQUAL
        (callable_signature<void (int, float)>, void (int, float));

      BOOST_REQUIRE ((is_callable<void(), void()>{}));
      BOOST_REQUIRE ((is_callable<void (void), void()>{}));
      BOOST_REQUIRE ((is_callable<void (int, float), void (int, float)>{}));
    }

    extern int some_function (char const*);

    BOOST_AUTO_TEST_CASE (function_pointer)
    {
      GSPC_TESTING_CHECK_TYPE_EQUAL
        (callable_signature<decltype (&some_function)>, int (char const*));
      GSPC_TESTING_CHECK_TYPE_EQUAL
        (callable_signature<int (*) (char const*)>, int (char const*));

      BOOST_REQUIRE
        ((is_callable<decltype (&some_function), int (char const*)>{}));
      BOOST_REQUIRE ((is_callable<int (char const*), int (char const*)>{}));
    }

    BOOST_AUTO_TEST_CASE (functor)
    {
      GSPC_TESTING_CHECK_TYPE_EQUAL
        (callable_signature<fun>, int (fun const&));

      BOOST_REQUIRE ((is_callable<fun, int (fun const&)>{}));
    }

    BOOST_AUTO_TEST_CASE (member_functions)
    {
      GSPC_TESTING_CHECK_TYPE_EQUAL
        ( callable_signature<decltype (&fun::member_function)>
        , callable_signature<decltype (&fun::static_function)>
        );
      GSPC_TESTING_CHECK_TYPE_EQUAL
        ( callable_signature<decltype (&fun::member_function)>
        , void (std::string&&)
        );

      BOOST_REQUIRE
        ((is_callable<decltype (&fun::static_function), void (std::string&&)>{}));
    }

    BOOST_AUTO_TEST_CASE (check_return_type)
    {
      GSPC_TESTING_CHECK_TYPE_EQUAL
        (return_type<fun>, int);
      GSPC_TESTING_CHECK_TYPE_EQUAL
        (return_type<decltype (&fun::member_function)>, void);
      GSPC_TESTING_CHECK_TYPE_EQUAL
        (return_type<decltype (&fun::static_function)>, void);
      GSPC_TESTING_CHECK_TYPE_EQUAL
        (return_type<decltype (&some_function)>, int);
    }
  }
