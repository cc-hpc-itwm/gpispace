#pragma once

#include <gspc/detail/counter/zero.hpp>

namespace gspc
{
  namespace detail
  {
    namespace counter
    {
      template<typename Counter>
        bool in_use (Counter c)
      {
        return c != zero<Counter>();
      }
    }
  }
}
