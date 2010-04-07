// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_TOKEN_HPP
#define _WE_TYPE_TOKEN_HPP

#include <we/expr/variant/variant.hpp>

#include <string>

#include <vector>

namespace we
{
  namespace token
  {
    typedef std::string field_name_t;
    typedef std::pair<field_name_t, expr::variant::type> field_t;

    typedef std::vector<field_t> type;

    static inline field_name_t name (const field_t & f)
    {
      return f.first;
    }

    static inline expr::variant::type value (const field_t & f)
    {
      return f.second;
    }
  }
}

#endif
