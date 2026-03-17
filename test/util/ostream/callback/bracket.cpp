#include <gspc/util/ostream/callback/bracket.hpp>
#include <gspc/util/ostream/callback/print.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>




      namespace gspc::util::ostream::callback
      {
        BOOST_AUTO_TEST_CASE (bracket_surrounds_with_given_separators)
        {
          BOOST_REQUIRE_EQUAL
            (print<int> (bracket<int> ('[', ']'), 0).string(), "[0]");
          BOOST_REQUIRE_EQUAL
            (print<int> (bracket<int> ("~|", "|~"), 1).string(), "~|1|~");
          BOOST_REQUIRE_EQUAL
            (print<int> (bracket<int> (0, 1), 8).string(), "081");
        }
      }
