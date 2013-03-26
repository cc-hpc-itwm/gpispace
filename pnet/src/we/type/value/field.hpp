// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_FIELD_HPP
#define _WE_TYPE_VALUE_FIELD_HPP

#include <we/type/value.hpp>

#include <we/type/signature.hpp>

namespace value
{
  type& field (const signature::field_name_t& field, type& v);
}

#endif
