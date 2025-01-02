// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <xml/parse/parser.hpp>

#include <util-generic/cxx17/holds_alternative.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random/string.hpp>

#include <fmt/core.h>
#include <list>
#include <sstream>

BOOST_AUTO_TEST_CASE (conditions_and_expressions_with_cdata)
{
  std::string const input
    ( fmt::format (R"EOS(
<defun name="{}">
<expression>${{b}} := ${{a}}
<![CDATA[${{c}} := ${{a}} + 1L]]>
${{d}} := 2L*${{a}};
${{another}} := ${{expression}}
</expression>
<condition><![CDATA[${{a}} < 10]]>
           <!-- huhu -->
           ${{a}} :gt: 20L
           <![CDATA[${{a}}]]>
</condition>
<condition>${{a}} :gt: 20L</condition>
</defun>)EOS"
                   , fhg::util::testing::random_identifier()
      )
    );

  xml::parse::state::type state;
  std::istringstream input_stream (input);
  xml::parse::type::function_type const function
    (xml::parse::just_parse (state, input_stream));

  std::list<std::string> const expected_conditions
    { "${a} < 10"
    , "\n           ${a} :gt: 20L\n           "
    , "${a}"
    , "${a} :gt: 20L"
    };

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    ( expected_conditions.begin(), expected_conditions.end()
    , function.conditions().begin(), function.conditions().end()
    );

  BOOST_REQUIRE (fhg::util::cxx17::holds_alternative<xml::parse::type::expression_type>
                  (function.content())
                );

  //! \note whitespace before second linebreak
  std::string const expected_expression
    ("${b} := ${a}\n; ${c} := ${a} + 1L; \n${d} := 2L*${a};\n${another} := ${expression}\n");

  BOOST_REQUIRE_EQUAL
    ( ::boost::get<xml::parse::type::expression_type> (function.content())
    . expression()
    , expected_expression
    );
}
