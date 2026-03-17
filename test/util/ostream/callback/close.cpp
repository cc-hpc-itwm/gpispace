#include <gspc/util/ostream/callback/close.hpp>
#include <gspc/util/ostream/callback/print.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>




      namespace gspc::util::ostream::callback
      {
        BOOST_AUTO_TEST_CASE (close_appends_given_separator)
        {
          BOOST_REQUIRE_EQUAL
            (print<int> (close<int> (']'), 0).string(), "0]");
          BOOST_REQUIRE_EQUAL
            (print<int> (close<int> ("|~"), 1).string(), "1|~");
          BOOST_REQUIRE_EQUAL
            (print<int> (close<int> (1), 8).string(), "81");
        }
      }
