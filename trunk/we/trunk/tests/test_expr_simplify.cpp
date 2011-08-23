// bernd.loerwald@itwm.fraunhofer.de

#include <iostream>
#include <string>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/expr/parse/util/get_names.hpp>

#include <we/expr/parse/simplify/ssa_tree.hpp>
#include <we/expr/parse/simplify/expression_list.hpp>
#include <we/expr/parse/simplify/numbering_and_propagation_pass.hpp>
#include <we/expr/parse/simplify/dead_code_elimination_pass.hpp>

static inline expr::parse::node::key_vec_t
key_vec_from_string (const std::string & in)
{
  expr::parse::node::key_vec_t result;
  boost::algorithm::split (result, in, boost::algorithm::is_any_of ("."));
  return result;
}

static inline void
add_to_bindings_list ( const std::string & name
                     , const expr::parse::simplify::tree_node_type & tree
                     , expr::parse::util::name_set_t & bindings
                     )
{
  bindings.insert( tree.get_ssa_name (key_vec_from_string (name)));
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
     "(${volume.id} := ${pair.id});\n"
     "(${volume.offset} := ${pair.tag});\n"
     "(${z.a} := []);\n"
     "(${z.b} := []);\n"
     "(${z} := []);\n"
     "(${z.a} := []);\n"
     "([]);\n"
    );

  expr::parse::parser parser (input);

  expr::parse::util::name_set_t names (expr::parse::util::get_names (parser));

  expr::parse::simplify::tree_node_type tree
    (expr::parse::simplify::create_from_name_set
      (names, expr::parse::simplify::line_type()));

  expr::parse::simplify::expression_list expressions (parser);

        // steps done:
        //  * add ssa numbering
        //  * copy and constant propagation

  expr::parse::simplify::numbering_and_propagation_pass (expressions, tree);

        // steps to be done:
        //  * dead-code elimination
        //  * copy propagation fixup

  expr::parse::util::name_set_t needed_bindings;
  add_to_bindings_list ("___loadTT_generate_init_state", tree, needed_bindings);
  add_to_bindings_list ("__generate_offset_credits_trigger_when_amount_state", tree, needed_bindings);
  add_to_bindings_list ("__generate_volume_credits_trigger_when_amount_state", tree, needed_bindings);
  add_to_bindings_list ("_extract_number_of_volume_credits_b", tree, needed_bindings);
  add_to_bindings_list ("state", tree, needed_bindings);
  add_to_bindings_list ("_init_wait_wait", tree, needed_bindings);
  add_to_bindings_list ("object", tree, needed_bindings);
  add_to_bindings_list ("wait", tree, needed_bindings);
  add_to_bindings_list ("b", tree, needed_bindings);
  add_to_bindings_list ("volume", tree, needed_bindings);
  add_to_bindings_list ("state", tree, needed_bindings);

  expr::parse::simplify::dead_code_elimination_pass
      (expressions, needed_bindings);

  expr::parse::parser result_parser (expressions.node_stack());

  std::cout << result_parser;

  return EXIT_SUCCESS;
}
