// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_TO_VALUE_HPP
#define PNET_SRC_WE_TYPE_VALUE_TO_VALUE_HPP

#include <we/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      template<typename T>
        inline value_type to_value (const T& x)
      {
        return x;
      }
    }
  }
}

#endif
