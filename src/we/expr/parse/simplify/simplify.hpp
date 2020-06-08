#pragma once

#include <we/expr/parse/parser.hpp>

#include <list>
#include <set>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      typedef std::set<std::list<std::string>> key_set_type;

      //! \todo also return the modified flag
      parser simplification_pass
        (const parser&, const key_set_type& needed_bindings);
    }
  }
}
