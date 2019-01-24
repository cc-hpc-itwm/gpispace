#pragma once

namespace gspc
{
  namespace detail
  {
    namespace counter
    {
      template<typename Counter>
        Counter zero()
      {
        return Counter{};
      }
    }
  }
}
