// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_FROM_VALUE_HPP
#define PNET_SRC_WE_TYPE_VALUE_FROM_VALUE_HPP

#include <we/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      template<typename T>
        inline T from_value (value_type const& v)
      {
        return boost::get<T> (v);
      }
    }
  }
}

#endif
