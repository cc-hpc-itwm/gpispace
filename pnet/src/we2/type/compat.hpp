// mirko.rahn@itwm.fhg.de

#ifndef PNET_SRC_WE_TYPE_COMPAT_HPP
#define PNET_SRC_WE_TYPE_COMPAT_HPP

#include <we/type/value.hpp>
#include <we2/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace compat
    {
      ::value::type COMPAT (const value::value_type&);
      value::value_type COMPAT (const ::value::type&);
    }
  }
}

#endif
