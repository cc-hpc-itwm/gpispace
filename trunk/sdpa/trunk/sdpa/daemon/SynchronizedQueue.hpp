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
#include <sdpa/SDPAException.hpp>

namespace sdpa { namespace daemon {
  class QueueException : public SDPAException
  {
    public:
      explicit
      QueueException(const std::string &a_reason) : SDPAException(a_reason) {} 
  };

  class QueueFull : public QueueException
  {
    public:
      QueueFull() : QueueException("queue is full") {}
  };

  class QueueEmpty : public QueueException
  {
    public:
      QueueEmpty() : QueueException("queue is empty") {}
  };

  /*
   * Implementation of a synchronized queue on top of a container.
   *
   * The required functions for the Container type are:
   *    size(), empty(), push_back(), pop_front(), front()
   *    Container::value_type
   */
  template <class Container> class SynchronizedQueue {
  public:
    typedef Container container_type;
    typedef typename container_type::value_type value_type;
    typedef typename container_type::reference reference;
    typedef typename container_type::const_reference const_reference;

    typedef typename container_type::size_type size_type;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;
    typedef boost::condition_variable_any condition_type;

    SynchronizedQueue()
      : mtx_()
      , not_empty_()
      , container_()
    {
    }
    ~SynchronizedQueue() {}
    
    inline value_type pop() throw (QueueEmpty)
    {
      lock_type lock(mtx_);
      if (container_.empty()) throw QueueEmpty();

      value_type item = container_.front();
      container_.pop_front();
      return item;
    }

    inline value_type pop_and_wait() throw (boost::thread_interrupted)
    {
      lock_type lock(mtx_);
      while (container_.empty())
      {
        not_empty_.wait(lock);
      }

      value_type item = container_.front();
      container_.pop_front();
      return item;
    }

    inline void push(const_reference item)
    {
      lock_type lock(mtx_);
      container_.push_back(item);
      not_empty_.notify_one();
    }

    inline size_type size() const
    {
      lock_type lock(mtx_);
      return container_.size();
    }

    inline bool empty() const
    {
      lock_type lock(mtx_);
      return container_.empty();
    }

    inline iterator erase(iterator first, iterator last)
    {
      lock_type lock(mtx_);
      return container_.erase(first, last);
    }

    inline iterator erase(iterator pos)
    {
      lock_type lock(mtx_);
      return container_.erase(pos);
    }

    inline iterator begin()
    {
      // WARN: make sure you hold a lock to the mutex!
      return container_.begin();
    }
    inline iterator end()
    {
      // WARN: make sure you hold a lock to the mutex!
      return container_.end();
    }

    inline void clear()
    {
      lock_type lock(mtx_);
      container_.clear();
    }

    mutex_type &mutex() { return mtx_; }
  private:
    mutable mutex_type mtx_;
    condition_type not_empty_;
    container_type container_;
  };
}}
#endif
