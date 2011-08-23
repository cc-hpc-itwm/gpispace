#ifndef FHG_UTIL_THREAD_QUEUE_HPP
#define FHG_UTIL_THREAD_QUEUE_HPP

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace fhg
{
  namespace thread
  {
    template < typename T
             , template < typename
                        , typename
                        > class Container
             , typename Allocator = std::allocator<T>
             >
    class queue
    {
      typedef boost::mutex              mutex;
      typedef boost::unique_lock<mutex> lock_type;
      typedef boost::condition_variable condition;

    public:
      typedef T value_type;
      typedef Container<T, Allocator> container_type;

      T get()
      {
        lock_type lock(m_mtx);
        while (m_container.empty()) m_cond.wait(lock);
        T t (m_container.front()); m_container.pop_front();
        return t;
      }

      void put(T t)
      {
        lock_type lock(m_mtx);
        m_container.push_back(t);
        m_cond.notify_one();
      }

    private:
      mutable mutex m_mtx;
      mutable condition m_cond;

      container_type m_container;
    };
  }
}

#endif
