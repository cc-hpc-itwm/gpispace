#include <gspc/util/ostream/to_string.hpp>
#include <gspc/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <sstream>



    namespace gspc::util::ostream
    {
      BOOST_AUTO_TEST_CASE (to_string_uses_id_by_default)
      {
        auto const value (gspc::testing::random<int>{}());
        BOOST_REQUIRE_EQUAL
          (ostream::to_string (value), std::to_string (value));
      }

      namespace
      {
        std::ostream& weird_int_printer (std::ostream& os, int const& i)
        {
          return os << "-" << -i;
        }
      }

      BOOST_AUTO_TEST_CASE (to_string_may_use_a_different_printer)
      {
        auto const value (gspc::testing::random<int>{}());
        BOOST_REQUIRE_EQUAL
          ( ostream::to_string
              (value, callback::print_function<int> (weird_int_printer))
          , "-" + std::to_string (-value)
          );
      }
    }
