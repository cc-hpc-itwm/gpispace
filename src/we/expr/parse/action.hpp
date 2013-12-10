// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_ACTION_HPP
#define _EXPR_PARSE_ACTION_HPP

#include <we/expr/token/type.hpp>

#include <iosfwd>

namespace expr
{
  namespace parse
  {
    namespace action
    {
      enum type
      { shift
      , reduce
      , accept
      , error1
      , error2
      , error3
      , error4
      };

      std::ostream& operator<< (std::ostream&, const type&);
      type action (const token::type&, const token::type&);
    }
  }
}

#endif
