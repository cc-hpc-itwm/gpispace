// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_REQUIRE_TYPE_HPP
#define PNET_SRC_WE_TYPE_VALUE_REQUIRE_TYPE_HPP

#include <we/type/value.hpp>
#include <we/type/value/signature.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      void require_type (const signature_type&, const value_type&);
    }
  }
}

#endif
