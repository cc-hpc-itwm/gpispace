#pragma once

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
