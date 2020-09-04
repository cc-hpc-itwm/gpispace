// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <we/expr/parse/parser.hpp>
#include <we/expr/parse/simplify/simplify.hpp>
#include <we/expr/parse/simplify/util.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/test/unit_test.hpp>

#include <string>

namespace
{
  expr::parse::simplify::key_type key_vec_from_string (const std::string& in)
  {
    expr::parse::simplify::key_type result;
    boost::algorithm::split (result, in, boost::algorithm::is_any_of ("."));
    return result;
  }

  void test ( const std::string& input
            , const std::string& expected_output
            , const std::list<std::string>& _needed_bindings
            )
  {
    expr::parse::simplify::key_set_type needed_bindings;

    for (const std::string& name : _needed_bindings)
    {
      needed_bindings.insert (key_vec_from_string (name));
    }

    const expr::parse::parser parser (input);

    const expr::parse::parser simplified_parser
      (expr::parse::simplify::simplification_pass (parser, needed_bindings));

    BOOST_REQUIRE_EQUAL (simplified_parser.string(), expected_output);
  }
}

BOOST_AUTO_TEST_CASE (constant_propagation)
{
  const std::string input
    ( "(${a} := []);"
      "(${b} := ${a});"
    );

  const std::string expected_output
    ( "(${a} := []);"
      "(${b} := []);"
    );

  std::list<std::string> needed_bindings;
  needed_bindings.push_back ("a");
  needed_bindings.push_back ("b");

  test (input, expected_output, needed_bindings);
}

BOOST_AUTO_TEST_CASE (constant_propagation_nothing_needed)
{
  const std::string input
    ( "(${a} := []);"
    );

  const std::string expected_output;

  std::list<std::string> needed_bindings;

  test (input, expected_output, needed_bindings);
}

BOOST_AUTO_TEST_CASE (constant_propagation_complex)
{
  const std::string input
    ( "(${a} := []);"
      "(${b} := ${a});"
      "(${a.a} := []);"
      "(${c} := ${a});"
    );

  const std::string expected_output
    ( "(${a} := []);"
      "(${b} := []);"
      "(${a.a} := []);"
      "(${c} := ${a});"
    );

  std::list<std::string> needed_bindings;
  needed_bindings.push_back ("b");
  needed_bindings.push_back ("c");

  test (input, expected_output, needed_bindings);
}

BOOST_AUTO_TEST_CASE (copy_propagation)
{
  const std::string input
    ( "(${b} := ${a});"
      "(${c} := ${b});"
    );

  const std::string expected_output
    ( "(${b} := ${a});"
      "(${c} := ${a});"
    );

  std::list<std::string> needed_bindings;
  needed_bindings.push_back ("b");
  needed_bindings.push_back ("c");

  test (input, expected_output, needed_bindings);
}

BOOST_AUTO_TEST_CASE (copy_propagation_complex)
{
  const std::string input
    ( "(${b.b.b} := ${a});"
      "(${c} := ${b.b.b.b});"
    );

  const std::string expected_output
    ( "(${c} := ${a.b});"
    );

  std::list<std::string> needed_bindings;
  needed_bindings.push_back ("c");

  test (input, expected_output, needed_bindings);
}

BOOST_AUTO_TEST_CASE (copy_propagation_complex_and_overwrite)
{
  const std::string input
    ( "(${b.b.b} := ${a});"
      "(${c} := ${b.b.b.b});"
      "(${b.b} := ${x});"
      "(${c} := ${b.b.b.b});"
    );

  const std::string expected_output
    ( "(${c} := ${x.b.b});"
    );

  std::list<std::string> needed_bindings;
  needed_bindings.push_back ("c");

  test (input, expected_output, needed_bindings);
}

BOOST_AUTO_TEST_CASE (constant_and_copy_propagation)
{
  const std::string input
    ( "(${a} := []);"
      "(${b} := ${a});"
      "(${c} := ${b});"
    );

  const std::string expected_output
    ( "(${a} := []);"
      "(${b} := []);"
      "(${c} := []);"
    );

  std::list<std::string> needed_bindings;
  needed_bindings.push_back ("a");
  needed_bindings.push_back ("b");
  needed_bindings.push_back ("c");

  test (input, expected_output, needed_bindings);
}

BOOST_AUTO_TEST_CASE (dead_code_elimination)
{
  const std::string input
    ( "(${a} := []);"
      "(${b} := []);"
      "([]);"
    );

  const std::string expected_output
    ("(${a} := []);");

  std::list<std::string> needed_bindings;
  needed_bindings.push_back ("a");

  test (input, expected_output, needed_bindings);
}

BOOST_AUTO_TEST_CASE (dead_code_elimination_complex)
{
  //! \todo can we do some elimination here?
  const std::string input
    ( "(${a.a.a} := (${a.a} + 1L));"
      "(${a.a.a} := []);"
    );

  const std::string expected_output
    ( input
    );

  std::list<std::string> needed_bindings;
  needed_bindings.push_back ("a");

  test (input, expected_output, needed_bindings);
}

BOOST_AUTO_TEST_CASE (all_combined)
{
  const std::string input
    ("(${trigger} := []);"
     "(${one} := ${trigger});"
     "(${two} := ${trigger});"
     "(${three} := ${trigger});"
     "(${four} := ${trigger});"
     "(${five} := ${trigger});"
     "(${parallel} := ${object.PARALLEL_LOADTT});"
     "(${wait} := ${parallel});"
     "(${offsets} := ${object.OFFSETS});"
     "(${state.id} := 0L);"
     "(${state.max} := ${offsets});"
     "(${N} := (2L + (${object.VOLUME_CREDITS} div ${object.SUBVOLUMES_PER_OFFSET})));"
     "(${a} := ${N});"
     "(${b} := ${N});"
     "(${trigger} := []);"
     "(${__generate_offset_credits_trigger_when_amount_state.pair.tag} := ${trigger});"
     "(${__generate_offset_credits_trigger_when_amount_state.pair.id} := 0L);"
     "(${__generate_offset_credits_trigger_when_amount_state.max} := ${a});"
     "(${___loadTT_generate_init_state.id} := 0L);"
     "(${___loadTT_generate_init_state.max} := ${parallel});"
     "(${_init_wait_wait} := (${object.OFFSETS} * ${object.SUBVOLUMES_PER_OFFSET}));"
     "(${N} := ${object.VOLUME_CREDITS});"
     "(${a} := ${N});"
     "(${_extract_number_of_volume_credits_b} := ${N});"
     "(${trigger} := []);"
     "(${__generate_volume_credits_trigger_when_amount_state.pair.tag} := ${trigger});"
     "(${__generate_volume_credits_trigger_when_amount_state.pair.id} := 0L);"
     "(${__generate_volume_credits_trigger_when_amount_state.max} := ${a});"
     "(${pair} := ${state.pair});"
     "(${state.pair.id} := (${state.pair.id} + 1L));"
     "(${aha} := ${state.pair});"
     "(${volume.id} := ${pair.id});"
     "(${b} := ${pair.id});"
     "(${test} := ${aha.id});"
     "(${volume.offset} := ${pair.tag});"
     "(${z.a} := []);"
     "(${z.b} := []);"
     "(${z} := []);"
     "(${z.a} := []);"
     "([]);"
    );

  const std::string expected_output
    ("(${wait} := ${object.PARALLEL_LOADTT});"
     "(${state.id} := 0L);"
     "(${state.max} := ${object.OFFSETS});"
     "(${N} := (2L + (${object.VOLUME_CREDITS} div ${object.SUBVOLUMES_PER_OFFSET})));"
     "(${__generate_offset_credits_trigger_when_amount_state.pair.tag} := []);"
     "(${__generate_offset_credits_trigger_when_amount_state.pair.id} := 0L);"
     "(${__generate_offset_credits_trigger_when_amount_state.max} := ${N});"
     "(${___loadTT_generate_init_state.id} := 0L);"
     "(${___loadTT_generate_init_state.max} := ${object.PARALLEL_LOADTT});"
     "(${_init_wait_wait} := (${object.OFFSETS} * ${object.SUBVOLUMES_PER_OFFSET}));"
     "(${_extract_number_of_volume_credits_b} := ${object.VOLUME_CREDITS});"
     "(${__generate_volume_credits_trigger_when_amount_state.pair.tag} := []);"
     "(${__generate_volume_credits_trigger_when_amount_state.pair.id} := 0L);"
     "(${__generate_volume_credits_trigger_when_amount_state.max} := ${object.VOLUME_CREDITS});"
     "(${pair} := ${state.pair});"
     "(${state.pair.id} := (${state.pair.id} + 1L));"
     "(${volume.id} := ${pair.id});"
     "(${b} := ${pair.id});"
     "(${test} := ${state.pair.id});"
     "(${volume.offset} := ${pair.tag});"
    );

  std::list<std::string> needed_bindings;
  needed_bindings.push_back ("___loadTT_generate_init_state");
  needed_bindings.push_back ("__generate_offset_credits_trigger_when_amount_state");
  needed_bindings.push_back ("__generate_volume_credits_trigger_when_amount_state");
  needed_bindings.push_back ("_extract_number_of_volume_credits_b");
  needed_bindings.push_back ("state");
  needed_bindings.push_back ("_init_wait_wait");
  needed_bindings.push_back ("object");
  needed_bindings.push_back ("wait");
  needed_bindings.push_back ("b");
  needed_bindings.push_back ("volume");
  needed_bindings.push_back ("state");
  needed_bindings.push_back ("test");

  test (input, expected_output, needed_bindings);
}
