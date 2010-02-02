// thread safe deque, with bounded depth, mirko.rahn@itwm.fraunhofer.de

#ifndef _DEQUE_HPP
#define _DEQUE_HPP

#include <deque>

#include <boost/thread.hpp>

template<typename T>
struct deque
{
public:
  typedef typename std::deque<T>::size_type size_type;

  deque (const size_type & _max) : max (_max) {}

private:
  size_type max;
  std::deque<T> q;
  boost::condition_variable cond_put;
  boost::condition_variable cond_get;
  boost::mutex mutex;
 
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

  bool empty (void) const { return q.empty(); }
  size_type size (void) const { return q.size(); }
};

#endif
