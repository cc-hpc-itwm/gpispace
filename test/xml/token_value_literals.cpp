// Copyright (C) 2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests that inline `<token><value>` elements are correctly
// parsed for all literal types through the XML parser,
// including arithmetic expressions in token values and
// multiple tokens on a single place.
//
// Token strings are parsed at the gspc::xml::parse level, then
// evaluated with `generic_we_parse + eval_all` — the same
// pipeline that `net_synthesize` uses internally for
// `we_net.put_value`.
//
// Literal types exercised:
//   string, long (with arithmetic), double, control,
//   char, bitset, bool, bytearray

#include <boost/test/unit_test.hpp>

#include <gspc/xml/parse/parser.hpp>
#include <gspc/xml/parse/type/function.hpp>
#include <gspc/xml/parse/type/net.hpp>
#include <gspc/xml/parse/type/place.hpp>
#include <gspc/xml/parse/util/weparse.hpp>

#include <gspc/we/type/value.hpp>
#include <gspc/testing/random/string.hpp>
#include <gspc/testing/printer/we/type/value.hpp>
#include <gspc/we/type/bytearray.hpp>
#include <gspc/we/type/bitsetofint.hpp>
#include <gspc/we/type/literal/control.hpp>

#include <fmt/core.h>

#include <algorithm>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace
{
  // Parse token expressions for a place of the given type
  // and evaluate each to a value_type.
  //
  [[nodiscard]] auto parse_token_values
    ( std::string const& type
    , std::vector<std::string> const& expressions
    ) -> std::vector<gspc::pnet::type::value::value_type>
  {
    auto const place_name
      { gspc::testing::random_identifier_without_leading_underscore()
      };

    auto const input
      { fmt::format
          ( "<defun name=\"token_literals\"><net>"
            "<place name=\"{}\" type=\"{}\">{}"
            "</place></net></defun>"
          , place_name
          , type
          , std::accumulate
            ( std::begin (expressions)
            , std::end (expressions)
            , std::string{}
            , [] (std::string tokens, std::string const& expression)
              {
                return tokens += fmt::format
                  ( "<token><value>{}</value></token>"
                  , expression
                  );
              }
            )
          )
      };

    auto state {gspc::xml::parse::state::type{}};
    auto input_stream {std::istringstream {input}};
    auto const function {gspc::xml::parse::just_parse (state, input_stream)};

    auto const place
      { ::boost::get<gspc::xml::parse::type::net_type> (function.content())
          .places().get (place_name)
      };

    if (!place)
    {
      throw std::logic_error
        { fmt::format ("place not found: {}", place_name)
        };
    }

    auto const& tokens {place->get().tokens};

    std::vector<gspc::pnet::type::value::value_type> values;
    values.reserve (tokens.size());

    std::transform
      ( std::begin (tokens)
      , std::end (tokens)
      , std::back_inserter (values)
      , [] (auto const& token)
        {
          return gspc::xml::parse::util::generic_we_parse (token, "test token")
            .eval_all();
        }
      );

    return values;
  }

  [[nodiscard]] auto parse_token_value
    ( std::string const& type
    , std::string const& expression
    ) -> gspc::pnet::type::value::value_type
  {
    auto const values = parse_token_values (type, {expression});

    if (values.size() != 1u)
    {
      throw std::logic_error {"expected exactly one token"};
    }

    return values.front();
  }
}

BOOST_AUTO_TEST_CASE (token_value_string_literal)
{
  auto const value {parse_token_value ("string", "\"a_string\"")};
  BOOST_CHECK_EQUAL (boost::get<std::string> (value), "a_string");
}

BOOST_AUTO_TEST_CASE (token_value_long_with_arithmetic)
{
  auto const value {parse_token_value ("long", "42L * 23L")};
  // 42 * 23 = 966
  BOOST_CHECK_EQUAL (boost::get<long> (value), 966L);
}

BOOST_AUTO_TEST_CASE (token_value_double_literal)
{
  auto const value {parse_token_value ("double", "47.11e-23")};
  BOOST_CHECK_EQUAL (boost::get<double> (value), 47.11e-23);
}

BOOST_AUTO_TEST_CASE (token_value_control_literal)
{
  auto const value {parse_token_value ("control", "[]")};
  BOOST_CHECK_NO_THROW (boost::get<gspc::we::type::literal::control> (value));
}

BOOST_AUTO_TEST_CASE (token_value_multiple_tokens_on_place)
{
  auto const values = parse_token_values ("control", {"[]", "[]"});

  BOOST_REQUIRE_EQUAL (values.size(), 2u);
  for (auto const& v : values)
  {
    BOOST_CHECK_NO_THROW (boost::get<gspc::we::type::literal::control> (v));
  }
}

BOOST_AUTO_TEST_CASE (token_value_char_literal)
{
  auto const value {parse_token_value ("char", "'c'")};
  BOOST_CHECK_EQUAL (boost::get<char> (value), 'c');
}

BOOST_AUTO_TEST_CASE (token_value_bitset_literal)
{
  // {4096} stores raw block 4096 = 1 << 12, so bit 12 is set.
  auto const value {parse_token_value ("bitset", "{4096}")};
  auto const& bitset (boost::get<gspc::pnet::type::bitsetofint::type> (value));
  BOOST_CHECK (bitset.is_element (12));
}

BOOST_AUTO_TEST_CASE (token_value_bool_literal)
{
  auto const value {parse_token_value ("bool", "true")};
  BOOST_CHECK_EQUAL (boost::get<bool> (value), true);
}

BOOST_AUTO_TEST_CASE (token_value_bytearray_literal)
{
  auto const value {parse_token_value ("bytearray", "y(0 127 255)")};
  auto const& bytes (boost::get<gspc::we::type::bytearray> (value));
  BOOST_CHECK_EQUAL (bytes.v().size(), 3u);
}
