#include <gspc/util/hard_integral_typedef.hpp>
#include <gspc/testing/printer/hard_integral_typedef.hpp>
#include <gspc/testing/random.hpp>

#include <gspc/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

FHG_UTIL_HARD_INTEGRAL_TYPEDEF (signed_t, int);
GSPC_HARD_INTEGRAL_TYPEDEF_LOG_VALUE_PRINTER (signed_t)

FHG_UTIL_HARD_INTEGRAL_TYPEDEF (unsigned_t, unsigned int);
GSPC_HARD_INTEGRAL_TYPEDEF_LOG_VALUE_PRINTER (unsigned_t)



    namespace gspc::testing
    {
      BOOST_AUTO_TEST_CASE (prints_the_same_as_underlying_type_signed)
      {
        auto const value (random<signed_t::underlying_type>{}());

        GSPC_TESTING_REQUIRE_PRINTED_AS
          (std::to_string (value), signed_t (value));
        GSPC_TESTING_REQUIRE_PRINTED_AS
          (std::to_string (-value), signed_t (-value));
      }

      BOOST_AUTO_TEST_CASE (prints_the_same_as_underlying_type_unsigned)
      {
        auto const value (random<unsigned_t::underlying_type>{}());

        GSPC_TESTING_REQUIRE_PRINTED_AS
          (std::to_string (value), unsigned_t (value));
      }
    }
