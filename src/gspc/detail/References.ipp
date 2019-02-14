#include <limits>
#include <stdexcept>

namespace gspc
{
  namespace detail
  {

#define TEMPLATE template<typename Counter>
#define REFERENCES References<Counter>

    TEMPLATE Counter REFERENCES::zero()
    {
      return Counter{};
    }
    TEMPLATE bool REFERENCES::in_use (Counter c)
    {
      return c != zero();
    }

    TEMPLATE REFERENCES::References()
        : _ {zero()}
    {}

    TEMPLATE bool REFERENCES::in_use() const
    {
      return in_use (_);
    }
    TEMPLATE bool REFERENCES::increment()
    {
      if (_ == std::numeric_limits<Counter>::max())
      {
        throw std::logic_error ("References::increment: Overflow");
      }

      return !in_use (_++);
    }
    TEMPLATE bool REFERENCES::decrement()
    {
      if (!in_use())
      {
        throw std::logic_error ("References::decrement: Not in use");
      }

      return !in_use (--_);
    }

#undef REFERENCES
#undef TEMPLATE
  }
}
