// Copyright (C) 2020,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/drts/drts_iml.hpp>
#include <gspc/drts/StreamSlot.hpp>

#include <gspc/we/type/value/show.hpp>
#include <gspc/we/type/value/to_value.hpp>

#include <gspc/testing/iml/random/AllocationHandle.hpp>
#include <gspc/testing/iml/random/MemoryRegion.hpp>

#include <gspc/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <regex>
#include <sstream>
#include <string>

#define REQUIRE_TOKEN_MATCHES_REGEX(regex_, token_...)            \
  do                                                              \
  {                                                               \
    auto const token (token_);                                    \
    std::ostringstream oss;                                       \
    oss << gspc::pnet::type::value::show (token);                       \
                                                                  \
    BOOST_TEST_CONTEXT ("token = " << oss.str())                  \
      BOOST_REQUIRE                                               \
        (std::regex_match (oss.str(), std::regex {regex_}));      \
  } while (false)

BOOST_AUTO_TEST_CASE (handle_is_wrapped_correctly)
{
  REQUIRE_TOKEN_MATCHES_REGEX
    ( "\"0x[0-9a-f]+\""
    , gspc::pnet::type::value::to_value
        (gspc::testing::random<gspc::iml::AllocationHandle>{}())
    );
}

#define RANGE_PATTERN                                             \
  "Struct \\[handle := Struct \\[name := "                        \
  "\"0x[0-9a-f]+\"\\], offset := [1-9][0-9]*UL"                  \
  ", size := [1-9][0-9]*UL\\]"

BOOST_AUTO_TEST_CASE (range_is_wrapped_correctly)
{
  REQUIRE_TOKEN_MATCHES_REGEX
    ( RANGE_PATTERN
    , gspc::pnet::type::value::to_value
        (gspc::testing::random<gspc::iml::MemoryRegion>{}())
    );
}

namespace
{
  // \todo Move to gspc::testing::random<char>::no_newline().
  std::string no_newline()
  {
    auto result (gspc::testing::random<char>::any());
    result.erase ('\n');
    return result;
  }
}

BOOST_AUTO_TEST_CASE (stream_slot_is_wrapped_correctly)
{
  REQUIRE_TOKEN_MATCHES_REGEX
    ( "Struct \\[meta := " RANGE_PATTERN
      ", data := " RANGE_PATTERN
      ", flag := '.', id := [0-9]+UL\\]"
    , gspc::pnet::type::value::to_value
        ( gspc::StreamSlot
            { gspc::testing::random
                <gspc::iml::MemoryRegion>{}()
            , gspc::testing::random
                <gspc::iml::MemoryRegion>{}()
              // Regex does not like newlines.
            , gspc::testing::random<char>{}
                (no_newline())
            , gspc::testing::random<std::size_t>{}()
            }
        )
    );
}
