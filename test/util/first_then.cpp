#include <boost/test/unit_test.hpp>

#include <gspc/util/first_then.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/random.hpp>

BOOST_AUTO_TEST_CASE (first_then_works)
{
  std::string const first {gspc::testing::random<std::string>{}()};
  std::string const then {gspc::testing::random<std::string>{}()};

  gspc::util::first_then<std::string> const first_then {first, then};

  BOOST_REQUIRE_EQUAL (first_then.string(), first);
  BOOST_REQUIRE_EQUAL (first_then.string(), then);
  BOOST_REQUIRE_EQUAL (first_then.string(), then);
}
