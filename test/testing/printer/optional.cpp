#include <gspc/testing/printer/optional.hpp>

#include <gspc/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (unset)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("Nothing", std::nullopt);
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("Nothing", std::optional<int>{});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("Nothing", std::optional<std::string>{});
}

BOOST_AUTO_TEST_CASE (set)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("Just 1", std::optional<int> {1});
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("Just foo", std::optional<std::string> {"foo"});
}
