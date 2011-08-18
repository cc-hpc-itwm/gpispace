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
    ("${x.a} * ${y.a} - ${z.coord.phi} + ${x.b} / sin (${t} + pi)");

  expr::parse::parser parser (input);

  expr::parse::util::name_set_t names
    (expr::parse::util::get_names (parser.front()));

  typedef expr::parse::util::name_set_t name_set_t;

  expr::parse::simplify::tree_node_type tree
    (expr::parse::simplify::create_from_name_set (names, expr::parse::simplify::line_type()));

  expr::parse::simplify::helper::dump (tree);

  return EXIT_SUCCESS;
}
