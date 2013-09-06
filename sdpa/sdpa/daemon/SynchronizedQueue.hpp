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
#include <iostream>
#include <sdpa/logging.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>


namespace sdpa { namespace daemon {
  class QueueException : public SDPAException
  {
    public:
      explicit
      QueueException(const std::string &reason) : SDPAException(reason) {}
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

  class NotFoundItem : public QueueException
   {
     public:
	  NotFoundItem(const std::string& str) : QueueException(std::string("The item ")+str+" was not found!") {}
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
    typedef typename container_type::size_type size_type;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;
    typedef boost::condition_variable_any condition_type;

    SynchronizedQueue() : stopped_(false) {}
    ~SynchronizedQueue() { }

    inline value_type front()
    {
    	lock_type lock(mtx_);
    	if (container_.empty()) throw QueueEmpty();

    	value_type item = container_.front();
    	return item;
    }

    inline value_type pop()
    {
      lock_type lock(mtx_);
      if (container_.empty()) throw QueueEmpty();

      value_type item = container_.front();
      container_.pop_front();
      return item;
    }

    inline value_type pop_back()
    {
    	lock_type lock(mtx_);
    	if (container_.empty()) throw QueueEmpty();

    	value_type item = container_.back();
    	container_.pop_back();
    	return item;
    }

    inline void stop()
    {
    	lock_type lock(mtx_);
    	stopped_ = true;
    	not_empty_.notify_all();
    }

    inline value_type pop_and_wait()
    {
      lock_type lock(mtx_);
      while (container_.empty() && !stopped_)
      {
        not_empty_.wait(lock);
      }
      if (stopped_) throw QueueEmpty();

      value_type item = container_.front();
      container_.pop_front();
      return item;
    }

    inline value_type pop_and_wait(const boost::posix_time::time_duration &timeout)
    {
      lock_type lock(mtx_);

      const boost::system_time to = boost::get_system_time() + timeout;

      while (container_.empty() && !stopped_)
      {
        not_empty_.timed_wait (lock, to);
        if (container_.empty())
          throw QueueEmpty(); // timedout
      }

      if (stopped_) throw QueueEmpty();

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

    inline iterator find(const value_type& item)
    {
    	lock_type lock(mtx_);
    	for( iterator iter=begin(); iter!=end(); iter++ )
    		if( *iter==item )
    			return iter;

    	return end();
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

    inline size_t erase(const value_type& item)
    {
    	lock_type lock(mtx_);
        size_t count(0);
        iterator iter (begin());
        while (iter != end())
        {
          if( item == *iter )
          {
            iter = erase(iter);
            ++count;
          }
          else
          {
            ++iter;
          }
        }
        return count;
    }

    inline iterator begin()
    {
      // WARN: make sure you hold a lock to the mutex!
      return container_.begin();
    }

    inline const_iterator begin() const
    {
      // WARN: make sure you hold a lock to the mutex!
      return container_.begin();
    }

    inline iterator end()
    {
      // WARN: make sure you hold a lock to the mutex!
      return container_.end();
    }
    inline const_iterator end() const
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

    template <class Archive>
  	void serialize(Archive& ar, const unsigned int)
  	{
      	ar & container_;
  	}

    friend class boost::serialization::access;

    void print()
    {
    	lock_type lock(mtx_);
    	unsigned int k = 0;
    	for( iterator it = begin(); it!= end(); it++, k++)
    	{
          DMLOG (TRACE, "   element "<<k<<": \""<<*it<<"\"");
    	}
    }

  private:
    mutable mutex_type mtx_;
    condition_type not_empty_;
    container_type container_;
    bool stopped_;
  };
}}
#endif
