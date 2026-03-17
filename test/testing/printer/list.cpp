#include <gspc/testing/printer/list.hpp>

#include <gspc/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS ("list ()", std::list<std::string>{});
  GSPC_TESTING_REQUIRE_PRINTED_AS ("list ()", std::list<int>{});
  GSPC_TESTING_REQUIRE_PRINTED_AS ("list ()", std::list<float>{});
}

BOOST_AUTO_TEST_CASE (one_element)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("list (foo)", std::list<std::string> {"foo"});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("list (1)", std::list<int> {1});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("list (1)", std::list<float> {1.0f});
}

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("list (foo, bar, baz)", std::list<std::string> {"foo", "bar", "baz"});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("list (1, 63, 4523)", std::list<int> {1, 63, 4523});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("list (1, 2, 16.3999996)", std::list<float> {1.0f, 2.0f, 16.4f});
}
