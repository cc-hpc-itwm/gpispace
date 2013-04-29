// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_REQUIRE_TYPE_HPP
#define PNET_SRC_WE_REQUIRE_TYPE_HPP

#include <we/type/value.hpp>
#include <we/type/signature.hpp>

namespace pnet
{
  void require_type ( const type::value::value_type&
                    , const type::signature::signature_type&
                    );
}

#endif
