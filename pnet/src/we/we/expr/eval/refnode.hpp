// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_REFNODE_HPP
#define _EXPR_EVAL_REFNODE_HPP

#include <we/expr/parse/node.hpp>
#include <we/expr/eval/context.hpp>

#include <list>
#include <string>

namespace expr
{
  namespace eval
  {
    parse::node::type refnode_value ( const context&
                                    , const std::list<std::string>&
                                    );
    parse::node::type refnode_name (const std::list<std::string>&);
  }
}

#endif
