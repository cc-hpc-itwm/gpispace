#pragma once

#include <gspc/detail/Cache/UniqEmpty/UnorderedSet.hpp>
#include <gspc/detail/Cache/UniqUnused/UnorderedSet.hpp>
#include <gspc/detail/References.hpp>

#include <condition_variable>
#include <mutex>
#include <unordered_map>

namespace gspc
{
  template<typename ID>  // O(1) empty, O(Epu) push, O(Epo) pop
    using UniqEmptyDefault = detail::Cache::UniqEmpty::UnorderedSet<ID>;
  template<typename T>   // O(1) empty, O(Upu) push, O(Upo) pop, O(Uer) erase
    using UniqUnusedDefault = detail::Cache::UniqUnused::UnorderedSet<T>;

  template< typename ID
          , typename T
          , typename Counter
          , template<typename> class UniqEmpty = UniqEmptyDefault
          , template<typename> class UniqUnused = UniqUnusedDefault
          >
    struct Cache
  {
    Cache (ID);               // O(1), [0..id)

    struct interrupted{};
    void interrupt();         // O(1), once called all methods throw

    struct Allocation
    {
      enum State {Empty, Assigned, Remembered};

      ID id;
      State state;
    };

    Allocation alloc (T);     // O(max {Epo,Uer,Upo}), maybe blocking
    void free (T);            // O(max {Epu,Upu})

    void remember (T);        // O(1)
    bool forget (T);          // O(Epu+Uer), true if the cache has changed

  protected:
    std::mutex _guard;
    bool _interrupted {false};
    std::condition_variable _grown_or_interrupted;

    Allocation _alloc (T, std::unique_lock<std::mutex>&);
    void _free (T, std::lock_guard<std::mutex> const&);
    void _remember (T, std::lock_guard<std::mutex> const&);
    bool _forget (T, std::lock_guard<std::mutex> const&);

  private:
    ID _max;
    ID _next {0};
    UniqEmpty<ID> _empty;
    UniqUnused<T> _unused;

    struct Entry;
    std::unordered_map<T, Entry> _known;
  };
}

#include <gspc/Cache.ipp>
