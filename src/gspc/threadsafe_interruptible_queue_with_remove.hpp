#pragma once

#include <util-generic/callable_signature.hpp>

#include <boost/optional.hpp>

#include <condition_variable>
#include <forward_list>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace gspc
{
  //! all operations amortized O(1)
  template<typename T, typename ID>
    struct threadsafe_interruptible_queue_with_remove
  {
    //! duplicate element: throw, queue unchanged
    void push (T, ID);

    //! empty: throw, queue unchanged
    //! return: oldest element (fifo)
    std::pair<T, ID> pop();

    //! unknown element: none, queue unchanged
    boost::optional<std::pair<T, ID>> remove (ID);

    //! interrupt all running and all future pop()
    struct Interrupted {};
    void interrupt();

  private:
    std::mutex _guard;
    bool _interrupted {false};
    std::condition_variable _elements_added_or_interrupted;

    using Elements = std::forward_list<std::pair<T, ID>>;
    using Position = typename Elements::const_iterator;
    using Positions = std::unordered_map<ID, Position>;

    Elements _elements;
    Positions _positions;
    Position _before {_elements.cbefore_begin()};

    //! Requires lock being held.
    boost::optional<std::pair<T, ID>> do_remove (ID) noexcept;
  };
}

#include <gspc/threadsafe_interruptible_queue_with_remove.ipp>
