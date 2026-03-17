#include <gspc/util/ostream/callback/open.hpp>
#include <gspc/util/ostream/callback/print.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>




      namespace gspc::util::ostream::callback
      {
        BOOST_AUTO_TEST_CASE (open_prepends_given_separator)
        {
          BOOST_REQUIRE_EQUAL
            (print<int> (open<int> ('['), 0).string(), "[0");
          BOOST_REQUIRE_EQUAL
            (print<int> (open<int> ("~|"), 1).string(), "~|1");
          BOOST_REQUIRE_EQUAL
            (print<int> (open<int> (0), 8).string(), "08");
        }
      }
