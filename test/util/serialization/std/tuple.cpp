#include <gspc/util/serialization/std/tuple.hpp>

#include <gspc/testing/printer/tuple.hpp>
#include <gspc/testing/require_serialized_to_id.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID ({}, std::tuple<>);
}

BOOST_AUTO_TEST_CASE (one_element)
{
  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID ({"foo"}, std::tuple<std::string>);
  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID ({1}, std::tuple<int>);
  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID ({1.0f}, std::tuple<float>);
}

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID
    (("foo", "bar", "baz"), std::tuple<std::string, std::string, std::string>);
  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID
    ((1, 63, 4523), std::tuple<int, int, int>);
  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID
    ((1.0f, 2.0f, 16.4f), std::tuple<float, float, float>);

  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID
    (("foo", 63, 16.4f), std::tuple<std::string, int, float>);
}
