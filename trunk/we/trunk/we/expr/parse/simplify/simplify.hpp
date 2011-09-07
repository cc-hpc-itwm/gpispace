// bernd.loerwald@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_SIMPLIFY_SIMPLIFY_HPP
#define _EXPR_PARSE_SIMPLIFY_SIMPLIFY_HPP 1

#include <we/expr/parse/parser.hpp>
#include <we/expr/parse/util/get_names.hpp>

#include <we/expr/parse/simplify/ssa_tree.hpp>
#include <we/expr/parse/simplify/expression_list.hpp>
#include <we/expr/parse/simplify/numbering_and_propagation_pass.hpp>
#include <we/expr/parse/simplify/dead_code_elimination_pass.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      inline util::name_set_t
      get_ssa_names (const util::name_set_t & n, ssa_tree_type & tree)
      {
        util::name_set_t ssa_names;

        for ( util::name_set_t::const_iterator it (n.begin()), end (n.end())
            ; it != end
            ; ++it
            )
        {
          ssa_names.insert (tree.get_current_name (*it));
        }

        return ssa_names;
      }

      inline parser
      simplification_pass ( const parser & parser
                          , const util::name_set_t & needed_bindings)
      {
        ssa_tree_type tree;

        expression_list expressions (parser);

        numbering_and_propagation_pass (expressions, tree);

        util::name_set_t needed_bindings_ssa
            (get_ssa_names (needed_bindings, tree));

        dead_code_elimination_and_unnumbering_pass
            (expressions, needed_bindings_ssa);

        return parse::parser (expressions.nodes());
      }
    }
  }
}

#endif
