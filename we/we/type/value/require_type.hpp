// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_REQUIRE_TYPE_HPP
#define _WE_TYPE_VALUE_REQUIRE_TYPE_HPP

#include <we/type/value.hpp>

#include <we/type/signature.hpp>

namespace value
{
  const type& require_type ( const signature::field_name_t&
                           , const signature::type&
                           , const value::type&
                           );
}

#endif
