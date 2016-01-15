#pragma once

#include <we/expr/parse/node.hpp>

#include <list>
#include <string>

namespace expr
{
  namespace eval
  {
    parse::node::type refnode_name (const std::list<std::string>&);
  }
}
