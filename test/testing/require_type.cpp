#include <gspc/testing/require_type.hpp>

#include <boost/test/unit_test.hpp>



    namespace gspc::testing
    {
      extern char const* some_function (int);

      BOOST_AUTO_TEST_CASE (equal_test_succeeds_if_matching_types)
      {
        GSPC_TESTING_CHECK_TYPE_EQUAL (int, int);
        GSPC_TESTING_REQUIRE_TYPE_EQUAL (int, int);

        using a_struct = struct { int a; };
        GSPC_TESTING_CHECK_TYPE_EQUAL (a_struct, a_struct);
        GSPC_TESTING_CHECK_TYPE_EQUAL (std::string, std::string);
        GSPC_TESTING_CHECK_TYPE_EQUAL
          ( equal_test_succeeds_if_matching_types
          , equal_test_succeeds_if_matching_types
          );
        GSPC_TESTING_CHECK_TYPE_EQUAL
          (decltype (&some_function), decltype (&some_function));
        using a_typedef = int;
        GSPC_TESTING_CHECK_TYPE_EQUAL (a_typedef, int);
      }

      BOOST_AUTO_TEST_CASE (ne_test_succeeds_if_mismatching_types)
      {
        GSPC_TESTING_CHECK_TYPE_NE (int, float);
        GSPC_TESTING_REQUIRE_TYPE_NE (int, float);

        using a_struct = struct { int a; };
        using b_struct = struct { int b; };
        GSPC_TESTING_CHECK_TYPE_NE (a_struct, b_struct);
        GSPC_TESTING_CHECK_TYPE_NE (std::string, std::size_t);
        GSPC_TESTING_CHECK_TYPE_NE
          ( equal_test_succeeds_if_matching_types
          , ne_test_succeeds_if_mismatching_types
          );
        GSPC_TESTING_CHECK_TYPE_NE
          (decltype (&some_function), std::string (int));
        using a_typedef = int;
        GSPC_TESTING_CHECK_TYPE_NE (a_typedef, float);
      }

#ifdef GSPC_TESTING_HAVE_CXXABI
      BOOST_AUTO_TEST_CASE (types_are_pretty_printed)
      {
        {
          std::ostringstream oss;
          detail::show_pretty_typename<int> (oss);
          BOOST_REQUIRE_EQUAL (oss.str(), "int");
        }

        {
          std::ostringstream oss;
          detail::show_pretty_typename<int (*)(float)> (oss);
          BOOST_REQUIRE_EQUAL (oss.str(), "int (*)(float)");
        }
      }
#endif
    }
