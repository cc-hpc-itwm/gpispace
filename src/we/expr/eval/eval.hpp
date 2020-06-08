#pragma once

#include <we/expr/eval/context.hpp>
#include <we/expr/parse/node.hpp>

namespace expr
{
  namespace eval
  {
    pnet::type::value::value_type eval (context&, const expr::parse::node::type&);
  }
}
