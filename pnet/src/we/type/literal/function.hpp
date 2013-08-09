// mirko.rahn@itwm.fraunhofer.de

#ifndef _TYPE_LITERAL_FUNCTION_HPP
#define _TYPE_LITERAL_FUNCTION_HPP

#include <we/expr/token/type.hpp>

#include <we/type/literal.hpp>

namespace literal
{
  namespace function
  {
    bool is_true (const literal::type&);

    literal::type unary (const expr::token::type&, literal::type );
    literal::type binary ( const expr::token::type&
                         , literal::type
                         , literal::type
                         );
  }
}

#endif
