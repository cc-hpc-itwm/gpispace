#pragma once

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

      template<>
        inline value_type from_value<> (value_type const& v)
      {
        return v;
      }
    }
  }
}
