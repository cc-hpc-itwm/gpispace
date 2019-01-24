#include <gspc/detail/counter/in_use.hpp>
#include <gspc/exception.hpp>

#include <utility>

namespace gspc
{
  namespace detail
  {
    template<typename Counter>
      CountDown<Counter>::CountDown (Counter counter)
        : _ {std::move (counter)}
    {
      if (!detail::counter::in_use (_))
      {
        INVALID_ARGUMENT ("CountDown::CountDown: Not in use.");
      }
    }
    template<typename Counter>
      bool CountDown<Counter>::decrement()
    {
      if (!detail::counter::in_use (_))
      {
        LOGIC_ERROR ("InputEntry::decrement: Not in use");
      }

      return !detail::counter::in_use (--_);
    }
  }
}
