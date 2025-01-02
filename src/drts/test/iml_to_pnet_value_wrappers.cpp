// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <drts/drts_iml.hpp>

#include <we/type/value/show.hpp>

#include <iml/testing/random/AllocationHandle.hpp>
#include <iml/testing/random/MemoryRegion.hpp>

#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <regex>
#include <sstream>
#include <string>

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
              (fhg::util::testing::random<iml::AllocationHandle>{}())
          );
      }

#define RANGE_PATTERN "Struct \\[handle := Struct \\[name := "          \
      "\"0x[0-9a-f]+\"\\], offset := [1-9][0-9]*UL, size := [1-9][0-9]*UL\\]"

      BOOST_AUTO_TEST_CASE (range_is_wrapped_correctly)
      {
        REQUIRE_TOKEN_MATCHES_REGEX
          ( RANGE_PATTERN
          , memory_region_to_value
              (fhg::util::testing::random<iml::MemoryRegion>{}())
          );
      }

      namespace
      {
        // \todo Move to fhg::util::testing::random<char>::no_newline().
        std::string no_newline()
        {
          auto result (fhg::util::testing::random<char>::any());
          result.erase ('\n');
          return result;
        }
      }

      BOOST_AUTO_TEST_CASE (stream_slot_is_wrapped_correctly)
      {
        REQUIRE_TOKEN_MATCHES_REGEX
          ( "Struct \\[meta := " RANGE_PATTERN ", data := " RANGE_PATTERN
            ", flag := '.', id := [0-9]+UL\\]"
          , stream_slot_to_value
              ( fhg::util::testing::random<iml::MemoryRegion>{}()
              , fhg::util::testing::random<iml::MemoryRegion>{}()
                // Regex does not like newlines.
              , fhg::util::testing::random<char>{} (no_newline())
              , fhg::util::testing::random<std::size_t>{}()
              )
          );
      }
    }
  }
}
