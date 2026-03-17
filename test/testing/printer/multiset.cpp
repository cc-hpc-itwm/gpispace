#include <gspc/testing/printer/multiset.hpp>

#include <gspc/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS ("multiset {}", std::multiset<std::string>{});
  GSPC_TESTING_REQUIRE_PRINTED_AS ("multiset {}", std::multiset<int>{});
  GSPC_TESTING_REQUIRE_PRINTED_AS ("multiset {}", std::multiset<float>{});
}

BOOST_AUTO_TEST_CASE (one_element)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("multiset {foo}", std::multiset<std::string> {"foo"});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("multiset {1}", std::multiset<int> {1});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("multiset {1}", std::multiset<float> {1.0f});
}

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("multiset {bar, baz, foo}", std::multiset<std::string> {"foo", "bar", "baz"});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("multiset {1, 63, 4523}", std::multiset<int> {1, 63, 4523});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("multiset {1, 2, 16.3999996}", std::multiset<float> {1.0f, 2.0f, 16.4f});
}

BOOST_AUTO_TEST_CASE (multiple_elements_same_key)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("multiset {bar, bar, bar}", std::multiset<std::string> {"bar", "bar", "bar"});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("multiset {63, 63, 63}", std::multiset<int> {63, 63, 63});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ( "multiset {16.3999996, 16.3999996, 16.3999996}"
    , std::multiset<float> {16.4f, 16.4f, 16.4f}
    );
}
