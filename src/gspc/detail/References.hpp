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
      Counter _;
    };
  }
}

#include <gspc/detail/References.ipp>
