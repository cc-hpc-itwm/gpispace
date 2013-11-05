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

    inline bool has_item (const value_type& item)
    {
       	lock_type lock(mtx_);
       	for( iterator iter=container_.begin(); iter!=container_.end(); iter++ )
       		if( *iter==item )
       			return true;

       	return false;
    }

    inline size_t erase(const value_type& item)
    {
    	lock_type lock(mtx_);
        size_t count(0);
        iterator iter (container_.begin());
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

    inline void clear()
    {
      lock_type lock(mtx_);
      container_.clear();
    }

    void print(const std::string& msg="")
    {
    	lock_type lock(mtx_);
    	std::ostringstream oss;
   		oss<<!msg.empty()?msg:"";
    	oss<<"{";
    	bool bFirst = true;
    	for(const_iterator it = container_.begin(); it != container_.end(); it++)
    	{
    		bFirst?bFirst=false:oss<<",";
    		oss<<it->str();
    	}
    	oss<<"}";

    	LOG(TRACE, oss.str());
    }

  private:
    mutable mutex_type mtx_;
    condition_type not_empty_;
    container_type container_;
    bool stopped_;
  };
}}
#endif
