#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>

namespace fhg
{
  namespace thread
  {
    template < typename T
             , template < typename
                        , typename
                        > class Container = std::deque
             , typename Allocator = std::allocator<T>
             >
    class queue
    {
    public:
      typedef std::mutex mutex_type;
      typedef std::unique_lock<mutex_type> lock_type;
      typedef std::condition_variable condition_type;

      typedef T value_type;
      typedef Container<T, Allocator> container_type;
      typedef typename container_type::size_type size_type;

      T get ()
      {
        lock_type lock (_mtx);

        while (_container.empty())
          {
            _cond.wait(lock);
          }

        T t (_container.front());

        _container.pop_front();

        return t;
      }

      void put (T const & t)
      {
        lock_type lock (_mtx);

        _container.push_back (t);

        _cond.notify_one();
      }

    private:
      mutable mutex_type _mtx;
      mutable condition_type _cond;

      container_type _container;
    };
  }
}
