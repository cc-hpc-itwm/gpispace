// bernd.loerwald@itwm.fraunhofer.de

#include <we/expr/parse/parser.hpp>

#include <we/expr/parse/util/get_names.hpp>
#include <we/expr/parse/simplify/ssa_tree.hpp>

#include <iostream>
#include <string>

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
      "(${volume.id} := ${pair.id});\n"
      "(${volume.offset} := ${pair.tag});\n");

  expr::parse::parser parser (input);

  expr::parse::util::name_set_t names (expr::parse::util::get_names (parser));

  typedef expr::parse::util::name_set_t name_set_t;

  expr::parse::simplify::tree_node_type tree
    (expr::parse::simplify::create_from_name_set (names, expr::parse::simplify::line_type()));

  expr::parse::simplify::helper::dump (tree);

  return EXIT_SUCCESS;
}
