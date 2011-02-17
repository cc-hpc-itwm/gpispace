// thread safe deque, with bounded depth, mirko.rahn@itwm.fraunhofer.de

#ifndef _CONCURRENT_DEQUE_HPP
#define _CONCURRENT_DEQUE_HPP

#include <deque>

#include <boost/thread.hpp>

namespace concurrent
{
  template<typename T, typename Alloc = std::allocator<T> >
  struct deque
  {
  public:
    typedef typename std::deque<T>::size_type size_type;

    deque (const size_type & _max) : max (_max) {}

  private:
    size_type max;
    std::deque<T, Alloc> q;
    boost::condition_variable cond_put;
    boost::condition_variable cond_get;
    boost::mutex mutex;

    bool empty (void) const
    {
      return q.empty();
    }
    size_type size (void) const
    {
      return q.size();
    }

  public:
    // maybe blocking!
    void put (const T & x)
    {
      boost::unique_lock<boost::mutex> lock (mutex);

      while (q.size() >= max)
        cond_get.wait (lock);

      q.push_back (x);

      cond_put.notify_one();
    }

    // maybe blocking!
    T get (void)
    {
      boost::unique_lock<boost::mutex> lock (mutex);

      while (q.empty())
        cond_put.wait (lock);

      const T x (q.front()); q.pop_front();

      cond_get.notify_one();

      return x;
    }
  };
}

#endif
