/*
 * =====================================================================================
 *
 *       Filename:  SynchronizedQueue.hpp
 *
 *    Description:  Implementation of a synchronized queue
 *
 *        Version:  1.0
 *        Created:  09/09/2009 12:17:06 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_DAEMON_SYNCHRONIZED_QUEUE_HPP
#define SDPA_DAEMON_SYNCHRONIZED_QUEUE_HPP 1

#include <boost/thread.hpp>

#include <iostream>
#include <stdexcept>

namespace sdpa { namespace daemon {
  /*
   * Implementation of a synchronized queue on top of a container.
   *
   * The required functions for the Container type are:
   *    size(), empty(), push_back(), pop_front(), front()
   *    Container::value_type
   */
  template <class Container> class SynchronizedQueue {
  public:
    typedef typename Container::value_type value_type;
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;

    inline value_type pop()
    {
      lock_type lock(mtx_);
      if (container_.empty()) throw std::runtime_error ("queue is empty");

      value_type item = container_.front();
      container_.pop_front();
      return item;
    }

    inline void push(value_type item)
    {
      lock_type lock(mtx_);
      container_.push_back(item);
      not_empty_.notify_one();
    }

    inline void push_front(value_type item)
    {
    	lock_type lock(mtx_);
    	container_.push_front(item);
    	not_empty_.notify_one();
    }

    inline bool empty() const
    {
      lock_type lock(mtx_);
      return container_.empty();
    }

    inline size_t erase(const value_type& item)
    {
    	lock_type lock(mtx_);
        size_t count(0);
        typename Container::iterator iter (container_.begin());
        while (iter != container_.end())
        {
          if( item == *iter )
          {
            iter = container_.erase(iter);
            ++count;
          }
          else
          {
            ++iter;
          }
        }
        return count;
    }

  private:
    mutable mutex_type mtx_;
    boost::condition_variable_any not_empty_;
    Container container_;
  };
}}
#endif
