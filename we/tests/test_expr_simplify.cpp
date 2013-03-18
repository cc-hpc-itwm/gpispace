// bernd.loerwald@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_expr_parse_simplify

#include <we/expr/parse/parser.hpp>
#include <we/expr/parse/simplify/simplify.hpp>
#include <we/expr/parse/simplify/util.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include <string>

namespace
{
  expr::parse::simplify::key_type key_vec_from_string (const std::string& in)
  {
    expr::parse::simplify::key_type result;
    boost::algorithm::split (result, in, boost::algorithm::is_any_of ("."));
    return result;
  }

  void add_to_bindings_list ( const std::string& name
                            , expr::parse::util::name_set_t& bindings
                            )
  {
    bindings.insert (key_vec_from_string (name));
  }

  void test ( const std::string& input
            , const std::string& expected_output
            , const std::list<std::string>& _needed_bindings
            )
  {
    expr::parse::util::name_set_t needed_bindings;

    BOOST_FOREACH (const std::string& name, _needed_bindings)
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

  expr::parse::util::name_set_t needed_bindings;
  add_to_bindings_list ("a", needed_bindings);
  add_to_bindings_list ("b", needed_bindings);

  expr::parse::parser parser (input);

  expr::parse::parser simplified_parser
    (expr::parse::simplify::simplification_pass (parser, needed_bindings));

  BOOST_REQUIRE_EQUAL (simplified_parser.string(), expected_output);
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

  expr::parse::util::name_set_t needed_bindings;
  add_to_bindings_list ("b", needed_bindings);
  add_to_bindings_list ("c", needed_bindings);

  expr::parse::parser parser (input);

  expr::parse::parser simplified_parser
    (expr::parse::simplify::simplification_pass (parser, needed_bindings));

  BOOST_REQUIRE_EQUAL (simplified_parser.string(), expected_output);
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

  expr::parse::util::name_set_t needed_bindings;
  add_to_bindings_list ("a", needed_bindings);
  add_to_bindings_list ("b", needed_bindings);
  add_to_bindings_list ("c", needed_bindings);

  expr::parse::parser parser (input);

  expr::parse::parser simplified_parser
    (expr::parse::simplify::simplification_pass (parser, needed_bindings));

  BOOST_REQUIRE_EQUAL (simplified_parser.string(), expected_output);
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

  expr::parse::util::name_set_t needed_bindings;
  add_to_bindings_list ("a", needed_bindings);

  expr::parse::parser parser (input);

  expr::parse::parser simplified_parser
    (expr::parse::simplify::simplification_pass (parser, needed_bindings));

  BOOST_REQUIRE_EQUAL (simplified_parser.string(), expected_output);
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
     "(${state.id} := 0);"
     "(${state.max} := ${object.OFFSETS});"
     "(${N} := (2 + (${object.VOLUME_CREDITS} div ${object.SUBVOLUMES_PER_OFFSET})));"
     "(${__generate_offset_credits_trigger_when_amount_state.pair.tag} := []);"
     "(${__generate_offset_credits_trigger_when_amount_state.pair.id} := 0);"
     "(${__generate_offset_credits_trigger_when_amount_state.max} := ${N});"
     "(${___loadTT_generate_init_state.id} := 0);"
     "(${___loadTT_generate_init_state.max} := ${object.PARALLEL_LOADTT});"
     "(${_init_wait_wait} := (${object.OFFSETS} * ${object.SUBVOLUMES_PER_OFFSET}));"
     "(${_extract_number_of_volume_credits_b} := ${object.VOLUME_CREDITS});"
     "(${__generate_volume_credits_trigger_when_amount_state.pair.tag} := []);"
     "(${__generate_volume_credits_trigger_when_amount_state.pair.id} := 0);"
     "(${__generate_volume_credits_trigger_when_amount_state.max} := ${object.VOLUME_CREDITS});"
     "(${pair} := ${state.pair});"
     "(${state.pair.id} := (${state.pair.id} + 1));"
     "(${volume.id} := ${pair.id});"
     "(${b} := ${pair.id});"
     "(${test} := ${state.pair.id});"
     "(${volume.offset} := ${pair.tag});"
    );

  expr::parse::util::name_set_t needed_bindings;
  add_to_bindings_list ("___loadTT_generate_init_state", needed_bindings);
  add_to_bindings_list ("__generate_offset_credits_trigger_when_amount_state", needed_bindings);
  add_to_bindings_list ("__generate_volume_credits_trigger_when_amount_state", needed_bindings);
  add_to_bindings_list ("_extract_number_of_volume_credits_b", needed_bindings);
  add_to_bindings_list ("state", needed_bindings);
  add_to_bindings_list ("_init_wait_wait", needed_bindings);
  add_to_bindings_list ("object", needed_bindings);
  add_to_bindings_list ("wait", needed_bindings);
  add_to_bindings_list ("b", needed_bindings);
  add_to_bindings_list ("volume", needed_bindings);
  add_to_bindings_list ("state", needed_bindings);
  add_to_bindings_list ("test", needed_bindings);

  expr::parse::parser parser (input);

  expr::parse::parser simplified_parser
    (expr::parse::simplify::simplification_pass (parser, needed_bindings));

  BOOST_REQUIRE_EQUAL (simplified_parser.string(), expected_output);
}
