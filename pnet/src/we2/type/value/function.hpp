// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_FUNCTION_HPP
#define PNET_SRC_WE_TYPE_VALUE_FUNCTION_HPP

#include <we2/type/value.hpp>

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
      bool is_true (const value_type&);
    }
  }
}

#endif
