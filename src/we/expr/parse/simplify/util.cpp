// bernd.loerwald@itwm.fraunhofer.de

#include <we/expr/parse/simplify/util.hpp>

#include <algorithm>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      namespace util
      {
        bool begins_with (const key_type& lhs, const key_type& rhs)
        {
          return lhs.size() >= rhs.size()
              && std::equal (rhs.begin(), rhs.end(), lhs.begin());
        }
      }
    }
  }
}
