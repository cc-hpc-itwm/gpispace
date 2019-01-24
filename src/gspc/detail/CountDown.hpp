#pragma once

namespace gspc
{
  namespace detail
  {
    template<typename Counter>
      struct CountDown
    {
      CountDown (Counter);

      bool decrement();       // returns: "is not in_use now"

    private:
      Counter _;
    };
  }
}

#include <gspc/detail/CountDown.ipp>
