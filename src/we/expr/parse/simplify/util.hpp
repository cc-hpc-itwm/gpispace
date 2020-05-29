#pragma once

#include <list>
#include <string>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      typedef std::list<std::string> key_type;

      namespace util
      {
        bool begins_with (const key_type& lhs, const key_type& rhs);
      }
    }
  }
}
