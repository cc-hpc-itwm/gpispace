// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <boost/test/unit_test.hpp>

#include <xml/parse/parser.hpp>

#include <fhg/util/boost/variant.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random/string.hpp>

#include <list>
#include <sstream>

BOOST_AUTO_TEST_CASE (conditions_and_expressions_with_cdata)
{
  std::string const input
    ( ( boost::format (R"EOS(
<defun name="%1%">
<expression>${b} := ${a}
<![CDATA[${c} := ${a} + 1L]]>
${d} := 2L*${a};
${another} := ${expression}
</expression>
<condition><![CDATA[${a} < 10]]>
           <!-- huhu -->
           ${a} :gt: 20L
           <![CDATA[${a}]]>
</condition>
<condition>${a} :gt: 20L</condition>
</defun>)EOS")
      % fhg::util::testing::random_identifier()
      ).str()
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

  BOOST_REQUIRE (fhg::util::boost::is_of_type<xml::parse::type::expression_type>
                  (function.content())
                );

  //! \note whitespace before second linebreak
  std::string const expected_expression
    ("${b} := ${a}\n; ${c} := ${a} + 1L; \n${d} := 2L*${a};\n${another} := ${expression}\n");

  BOOST_REQUIRE_EQUAL
    ( boost::get<xml::parse::type::expression_type> (function.content())
    . expression()
    , expected_expression
    );
}
