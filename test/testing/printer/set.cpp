#include <gspc/testing/printer/set.hpp>

#include <gspc/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS ("set {}", std::set<std::string>{});
  GSPC_TESTING_REQUIRE_PRINTED_AS ("set {}", std::set<int>{});
  GSPC_TESTING_REQUIRE_PRINTED_AS ("set {}", std::set<float>{});
}

BOOST_AUTO_TEST_CASE (one_element)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("set {foo}", std::set<std::string> {"foo"});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("set {1}", std::set<int> {1});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("set {1}", std::set<float> {1.0f});
}

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("set {bar, baz, foo}", std::set<std::string> {"foo", "bar", "baz"});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("set {1, 63, 4523}", std::set<int> {1, 63, 4523});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("set {1, 2, 16.3999996}", std::set<float> {1.0f, 2.0f, 16.4f});
}
