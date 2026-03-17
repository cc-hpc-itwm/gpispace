#include <gspc/util/mp/exactly_one_is.hpp>
#include <gspc/util/mp/none_is.hpp>

#include <boost/test/unit_test.hpp>



    namespace gspc::util::mp
    {
      BOOST_AUTO_TEST_CASE (none_is_missing)
      {
        BOOST_REQUIRE ((none_is<int>::value));
        BOOST_REQUIRE ((none_is<int, char>::value));
        BOOST_REQUIRE ((none_is<int, float, char>::value));
      }
      BOOST_AUTO_TEST_CASE (none_is_present)
      {
        BOOST_REQUIRE ((!none_is<int, int>::value));
        BOOST_REQUIRE ((!none_is<int, float, int>::value));
        BOOST_REQUIRE ((!none_is<int, int, float>::value));
      }

      BOOST_AUTO_TEST_CASE (exactly_one_is_one_present)
      {
        BOOST_REQUIRE ((exactly_one_is<int, int>::value));
        BOOST_REQUIRE ((exactly_one_is<int, int, char>::value));
        BOOST_REQUIRE ((exactly_one_is<int, char, int>::value));
      }
      BOOST_AUTO_TEST_CASE (exactly_one_is_none_present)
      {
        BOOST_REQUIRE ((!exactly_one_is<int>::value));
        BOOST_REQUIRE ((!exactly_one_is<int, char>::value));
      }
      BOOST_AUTO_TEST_CASE (exactly_one_is_multiple_present)
      {
        BOOST_REQUIRE ((!exactly_one_is<int, int, int>::value));
      }
    }
