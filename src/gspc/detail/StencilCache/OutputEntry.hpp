#pragma once

#include <gspc/detail/CountDown.hpp>

#include <list>
#include <utility>

namespace gspc
{
  namespace detail
  {
    namespace StencilCache
    {
      template<typename Output, typename Input, typename Slot, typename Counter>
        struct OutputEntry
      {
        using Assigned = std::pair<Slot, Input>;
        using Assignment = std::list<Assigned>;

        OutputEntry (Counter);

        bool prepared();
        Assignment const& assignment() const;
        void assigned (Slot, Input);

      private:
        CountDown<Counter> _count_down;
        Assignment _assignment;
      };
    }
  }
}

#include <gspc/detail/StencilCache/OutputEntry.ipp>
