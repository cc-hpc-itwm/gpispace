#include <gspc/testing/printer/vector.hpp>

#include <gspc/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS ("vector ()", std::vector<std::string>{});
  GSPC_TESTING_REQUIRE_PRINTED_AS ("vector ()", std::vector<int>{});
  GSPC_TESTING_REQUIRE_PRINTED_AS ("vector ()", std::vector<float>{});
}

BOOST_AUTO_TEST_CASE (one_element)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("vector (foo)", std::vector<std::string> {"foo"});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("vector (1)", std::vector<int> {1});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("vector (1)", std::vector<float> {1.0f});
}

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("vector (foo, bar, baz)", std::vector<std::string> {"foo", "bar", "baz"});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("vector (1, 63, 4523)", std::vector<int> {1, 63, 4523});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("vector (1, 2, 16.3999996)", std::vector<float> {1.0f, 2.0f, 16.4f});
}
