// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <we/type/value.hpp>

#include <we/expr/token/type.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      value_type unary (const expr::token::type&, const value_type&);
      value_type binary ( const expr::token::type&
                        , const value_type&
                        , const value_type&
                        );
    }
  }
}
