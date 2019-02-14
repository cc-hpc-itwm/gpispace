#pragma once

namespace gspc
{
  namespace detail
  {
    template<typename Counter>
      struct References
    {
      References();

      bool in_use() const;
      bool increment();       // returns: "was not in_use before"
      bool decrement();       // returns: "is not in_use now"

    private:
      static Counter zero();
      static bool in_use (Counter);

      Counter _;
    };
  }
}

#include <gspc/detail/References.ipp>
