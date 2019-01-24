#pragma once

#include <gspc/Cache.hpp>
#include <gspc/detail/StencilCache/InputEntry.fwd.hpp>
#include <gspc/detail/StencilCache/OutputEntry.fwd.hpp>

#include <functional>
#include <list>
#include <unordered_map>
#include <utility>

namespace gspc
{
  template< typename Output
          , typename Input
          , typename Slot
          , typename Counter
          , template<typename> class UniqEmpty = UniqEmptyDefault
          , template<typename> class UniqUnused = UniqUnusedDefault
          >
    struct StencilCache
      : private Cache<Slot, Input, Counter, UniqEmpty, UniqUnused>
  {
    using InputEntry = detail::StencilCache::InputEntry<Output, Counter>;
    using InputEntries = std::unordered_map<Input, InputEntry>;
    using Assigned = std::pair<Slot, Input>;
    using Assignment = std::list<Assigned>;
    using Inputs = std::list<Input>;
    using Prepare = std::function<void (Slot, Input)>;
    using Ready = std::function<void (Output, Assignment)>;

    StencilCache ( InputEntries
                 , Prepare         // maybe called by alloc
                 , Ready           // maybe called by alloc and by prepared
                 , Slot            // [0..Slot)
                 );                // O(1)


    struct interrupted{};
    void interrupt();              // O(1), once called all methods throw

    void alloc (Output, Inputs);   // O(#inputs * Base::alloc), maybe blocking
    void prepared (Input);         // O(#waiting outputs)
    void free (Input);             // O(Base::free + Base::forget)

  private:
    using Base = Cache<Slot, Input, Counter, UniqEmpty, UniqUnused>;
    using OutputEntry =
      detail::StencilCache::OutputEntry<Output, Input, Slot, Counter>;

    Prepare _prepare;
    Ready _ready;

    std::unordered_map<Input, InputEntry> _inputs;
    std::unordered_map<Output, OutputEntry> _outputs;

  public:
    void alloc (std::pair<Output, Inputs>);
  };
}

#include <gspc/StencilCache.ipp>
