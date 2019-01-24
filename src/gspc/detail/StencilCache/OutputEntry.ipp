#include <utility>

namespace gspc
{
  namespace detail
  {
    namespace StencilCache
    {
#define TEMPLATE template< typename Output      \
                         , typename Input       \
                         , typename Slot        \
                         , typename Counter     \
                         >
#define ENTRY OutputEntry<Output, Input, Slot, Counter>

      TEMPLATE ENTRY::OutputEntry (Counter counter)
        : _count_down {std::move (counter)}
      {}
      TEMPLATE bool ENTRY::prepared()
      {
        return _count_down.decrement();
      }
      TEMPLATE typename ENTRY::Assignment const& ENTRY::assignment() const
      {
        return _assignment;
      }
      TEMPLATE void ENTRY::assigned (Slot slot, Input i)
      {
        _assignment.emplace_back (Assigned {slot, i});
      }

#undef ENTRY
#undef TEMPLATE
    }
  }
}
