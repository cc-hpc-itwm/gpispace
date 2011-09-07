// bernd.loerwald@itwm.fraunhofer.de

#include <iostream>
#include <string>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/expr/parse/util/get_names.hpp>

#include <we/expr/parse/simplify/simplify.hpp>

static inline expr::parse::simplify::key_type
key_vec_from_string (const std::string & in)
{
  expr::parse::simplify::key_type result;
  boost::algorithm::split (result, in, boost::algorithm::is_any_of ("."));
  return result;
}

static inline void
add_to_bindings_list ( const std::string & name
                     , expr::parse::util::name_set_t & bindings
                     )
{
  bindings.insert (key_vec_from_string (name));
}

int
main (int argc, char ** argv)
{
  const std::string input
    ("(${trigger} := []);\n"
     "(${one} := ${trigger});\n"
     "(${two} := ${trigger});\n"
     "(${three} := ${trigger});\n"
     "(${four} := ${trigger});\n"
     "(${five} := ${trigger});\n"
     "(${parallel} := ${object.PARALLEL_LOADTT});\n"
     "(${wait} := ${parallel});\n"
     "(${offsets} := ${object.OFFSETS});\n"
     "(${state.id} := 0L);\n"
     "(${state.max} := ${offsets});\n"
     "(${N} := (2L + (${object.VOLUME_CREDITS} div ${object.SUBVOLUMES_PER_OFFSET})));\n"
     "(${a} := ${N});\n"
     "(${b} := ${N});\n"
     "(${trigger} := []);\n"
     "(${__generate_offset_credits_trigger_when_amount_state.pair.tag} := ${trigger});\n"
     "(${__generate_offset_credits_trigger_when_amount_state.pair.id} := 0L);\n"
     "(${__generate_offset_credits_trigger_when_amount_state.max} := ${a});\n"
     "(${___loadTT_generate_init_state.id} := 0L);\n"
     "(${___loadTT_generate_init_state.max} := ${parallel});\n"
     "(${_init_wait_wait} := (${object.OFFSETS} * ${object.SUBVOLUMES_PER_OFFSET}));\n"
     "(${N} := ${object.VOLUME_CREDITS});\n"
     "(${a} := ${N});\n"
     "(${_extract_number_of_volume_credits_b} := ${N});\n"
     "(${trigger} := []);\n"
     "(${__generate_volume_credits_trigger_when_amount_state.pair.tag} := ${trigger});\n"
     "(${__generate_volume_credits_trigger_when_amount_state.pair.id} := 0L);\n"
     "(${__generate_volume_credits_trigger_when_amount_state.max} := ${a});\n"
     "(${pair} := ${state.pair});\n"
     "(${state.pair.id} := (${state.pair.id} + 1L));\n"
     "(${aha} := ${state.pair});\n"
     "(${volume.id} := ${pair.id});\n"
     "(${b} := ${pair.id});\n"
     "(${test} := ${aha.id});\n"
     "(${volume.offset} := ${pair.tag});\n"
     "(${z.a} := []);\n"
     "(${z.b} := []);\n"
     "(${z} := []);\n"
     "(${z.a} := []);\n"
     "([]);\n"
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

  int passed = 0;

  try
  {
    expr::parse::parser simplified_parser
        (expr::parse::simplify::simplification_pass (parser, needed_bindings));

    if (simplified_parser.string() != expected_output)
    {
      std::cout << "result:\n" << simplified_parser.string() << "\n";
      std::cout << "expected:\n" << expected_output << "\n";
      --passed;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "exception during simplification: " << e.what() << std::endl;
  }


  return passed;
}
