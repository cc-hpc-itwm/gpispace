// mirko.rahn@itwm.fraunhofer.de

#ifndef _TYPE_VALUE_FUNCTION_HPP
#define _TYPE_VALUE_FUNCTION_HPP

#include <we/type/value.hpp>
#include <we/type/literal.hpp>
#include <we/type/literal/function.hpp>

#include <fhg/util/show.hpp>

#include <we2/type/compat.hpp>

namespace value
{
  namespace function
  {
    value::type unary (const expr::token::type&, value::type );
    value::type binary (const expr::token::type&, value::type , value::type );

    bool is_true (const type &);

    inline pnet::type::value::value_type unary ( const expr::token::type& t
                                               , pnet::type::value::value_type v
                                               )
    {
      return pnet::type::compat::COMPAT
        ( unary ( t
                , pnet::type::compat::COMPAT (v)
                )
        );
    }
    inline pnet::type::value::value_type binary ( const expr::token::type& t
                                                , pnet::type::value::value_type l
                                                , pnet::type::value::value_type r
                                                )
    {
      return pnet::type::compat::COMPAT
        ( binary ( t
                 , pnet::type::compat::COMPAT (l)
                 , pnet::type::compat::COMPAT (r)
                 )
        );
    }

    inline bool is_true (const pnet::type::value::value_type& v)
    {
      return is_true (pnet::type::compat::COMPAT (v));
    }
  }
}

#endif
