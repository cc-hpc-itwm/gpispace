#include <gspc/detail/counter/in_use.hpp>
#include <gspc/detail/counter/zero.hpp>
#include <gspc/exception.hpp>

#include <limits>

namespace gspc
{
  namespace detail
  {
    template<typename Counter>
      References<Counter>::References()
        : _ {counter::zero<Counter>()}
    {}

    template<typename Counter>
      bool References<Counter>::in_use() const
    {
      return counter::in_use (_);
    }
    template<typename Counter>
      bool References<Counter>::increment()
    {
      if (_ == std::numeric_limits<Counter>::max())
      {
        LOGIC_ERROR ("References::increment: Overflow");
      }

      return !counter::in_use (_++);
    }
    template<typename Counter>
      bool References<Counter>::decrement()
    {
      if (!in_use())
      {
        LOGIC_ERROR ("References::decrement: Not in use");
      }

      return !counter::in_use (--_);
    }
  }
}
