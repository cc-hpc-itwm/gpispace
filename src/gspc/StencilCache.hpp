#pragma once

#include <gspc/Cache.hpp>
#include <gspc/detail/References.hpp>

#include <functional>
#include <list>
#include <unordered_map>
#include <unordered_set>
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
    struct InputEntry
    {
      void increment();

    private:
      friend StencilCache;

      bool free();
      std::unordered_set<Output> const& waiting() const;
      void triggers (Output);
      void prepared();

      std::unordered_set<Output> _waiting;
      detail::References<Counter> _references;
    };

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
    struct OutputEntry;

    Prepare _prepare;
    Ready _ready;

    std::unordered_map<Input, InputEntry> _inputs;
    std::unordered_map<Output, OutputEntry> _outputs;

  public:
    void alloc (std::pair<Output, Inputs>);
  };
}

#include <gspc/StencilCache.ipp>
