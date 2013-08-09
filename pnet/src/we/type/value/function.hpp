// mirko.rahn@itwm.fraunhofer.de

#ifndef _TYPE_VALUE_FUNCTION_HPP
#define _TYPE_VALUE_FUNCTION_HPP

#include <we/type/value.hpp>
#include <we/type/literal.hpp>
#include <we/type/literal/function.hpp>

#include <fhg/util/show.hpp>

namespace value
{
  namespace function
  {
    value::type unary (const expr::token::type&, value::type );
    value::type binary (const expr::token::type&, value::type , value::type );

    bool is_true (const type &);
  }
}

#endif
