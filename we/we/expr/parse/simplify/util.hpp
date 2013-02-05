// bernd.loerwald@itwm.fraunhofer.de

#ifndef WE_EXPR_PARSE_SIMPLIFY_UTIL_HPP
#define WE_EXPR_PARSE_SIMPLIFY_UTIL_HPP

#include <we/type/value/container.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      typedef ::value::container::key_vec_t key_type;

      namespace util
      {
        bool begins_with (const key_type& lhs, const key_type& rhs);
      }
    }
  }
}

#endif
