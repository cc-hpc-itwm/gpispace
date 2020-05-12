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
      Counter increment();    // returns: reference count *including* this
      bool decrement();       // returns: "is not in_use now"

    private:
      static Counter zero();
      static bool in_use (Counter);

      Counter _;
    };
  }
}

#include <gspc/detail/References.ipp>
