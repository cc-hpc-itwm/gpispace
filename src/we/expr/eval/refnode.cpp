#include <we/expr/eval/refnode.hpp>

namespace expr
{
  namespace eval
  {
    parse::node::type refnode_name (const std::list<std::string>& key_vec)
    {
      return parse::node::type (key_vec);
    }
  }
}
