#include <drts/drts_iml.hpp>

#include <we/type/value/show.hpp>

#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <regex>
#include <sstream>

FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE (gpi::pc::type::range_t)
{
  return { random<gpi::pc::type::handle_id_t>{}()
         , random<gpi::pc::type::offset_t>{}()
         , random<gpi::pc::type::size_t>{}()
         };
}

namespace gspc
{
  namespace pnet
  {
    namespace vmem
    {
#define REQUIRE_TOKEN_MATCHES_REGEX(regex_, token_...)            \
      do                                                          \
      {                                                           \
        auto const token (token_);                                \
        std::ostringstream oss;                                   \
        oss << ::pnet::type::value::show (token);                 \
                                                                  \
        BOOST_TEST_CONTEXT ("token = " << oss.str())              \
          BOOST_REQUIRE                                           \
            (std::regex_match (oss.str(), std::regex {regex_}));  \
      } while (false)

      BOOST_AUTO_TEST_CASE (handle_is_wrapped_correctly)
      {
        REQUIRE_TOKEN_MATCHES_REGEX
          ("\"0x[0-9a-f]+\""
          , handle_to_value
              (fhg::util::testing::random<gpi::pc::type::handle_id_t>{}())
          );
      }

#define RANGE_PATTERN "Struct \\[handle := Struct \\[name := "          \
      "\"0x[0-9a-f]+\"\\], offset := [1-9][0-9]*UL, size := [1-9][0-9]*UL\\]"

      BOOST_AUTO_TEST_CASE (range_is_wrapped_correctly)
      {
        REQUIRE_TOKEN_MATCHES_REGEX
          ( RANGE_PATTERN
          , range_to_value
              (fhg::util::testing::random<gpi::pc::type::range_t>{}())
          );
      }

      BOOST_AUTO_TEST_CASE (stream_slot_is_wrapped_correctly)
      {
        REQUIRE_TOKEN_MATCHES_REGEX
          ( "Struct \\[meta := " RANGE_PATTERN ", data := " RANGE_PATTERN
            ", flag := '.', id := [0-9]+UL\\]"
          , stream_slot_to_value
              ( fhg::util::testing::random<gpi::pc::type::range_t>{}()
              , fhg::util::testing::random<gpi::pc::type::range_t>{}()
              , fhg::util::testing::random<char>{}()
              , fhg::util::testing::random<std::size_t>{}()
              )
          );
      }
    }
  }
}
