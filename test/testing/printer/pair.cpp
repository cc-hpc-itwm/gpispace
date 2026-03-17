#include <gspc/testing/printer/pair.hpp>

#include <gspc/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("<foo, bar>", std::make_pair ("foo", "bar"));
  GSPC_TESTING_REQUIRE_PRINTED_AS ("<1, 63>", std::make_pair (1, 63));
  GSPC_TESTING_REQUIRE_PRINTED_AS ("<1, 2>", std::make_pair (1.0f, 2.0f));

  GSPC_TESTING_REQUIRE_PRINTED_AS
    ("<foo, 16.3999996>", std::make_pair ("foo", 16.4f));
}
