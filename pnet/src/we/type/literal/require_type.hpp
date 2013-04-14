// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_REQUIRE_TYPE_HPP
#define _WE_TYPE_LITERAL_REQUIRE_TYPE_HPP

#include <we/type/signature.hpp>
#include <we/type/literal.hpp>
#include <we/type/literal/name.hpp>

namespace literal
{
  const type& require_type ( const std::string& field
                           , const type_name_t& req
                           , const type& x
                           );
}

#endif
