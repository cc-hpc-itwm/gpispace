#ifndef WE_MGMT_LAYER_BITS_HPP
#define WE_MGMT_LAYER_BITS_HPP 1

/*
 * =====================================================================================
 *
 *       Filename:  queue.hpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  03/10/2010 11:28:22 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include <boost/circular_buffer.hpp>
#include <boost/call_traits.hpp>
#include <boost/progress.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>

namespace we
{
  namespace mgmt
  {
    namespace detail
    {
      template <typename T, unsigned long long CAPACITY>
      class queue : boost::noncopyable
      {
        typedef queue<T, CAPACITY> this_type;
      public:
	typedef boost::circular_buffer<T> container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::value_type value_type;
	typedef typename boost::call_traits<value_type>::param_type param_type;

	queue() : unread_(0), container_(CAPACITY) {}

	void put (param_type item)
	{
	  boost::mutex::scoped_lock lock( mutex_ );
          while (! is_not_full())
          {
            not_full_.wait (lock);
          }

	  container_.push_front (item);
	  ++unread_;
	  not_empty_.notify_one();
	}

	value_type get ()
	{
	  value_type v;
	  get(&v);
	  return v;
	}

	void get (value_type *item)
	{
	  boost::mutex::scoped_lock lock( mutex_ );
          while (! is_not_empty())
          {
            not_empty_.wait (lock);
          }

	  *item = container_[--unread_];
	  not_full_.notify_one();
	}

        size_type size (void) const
        {
	  boost::mutex::scoped_lock lock( mutex_ );
          return unread_;
        }

      private:
	inline bool is_not_empty() const { return unread_ > 0; }
	inline bool is_not_full() const { return unread_ < CAPACITY; }

	size_type unread_;
	container_type container_;
	boost::mutex mutex_;
	boost::condition not_empty_;
	boost::condition not_full_;
      };

      template <typename T>
      class queue<T, 0> : boost::noncopyable
      {
      private:
        typedef queue<T, 0> this_type;
      public:
	typedef std::deque<T> container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::value_type value_type;
	typedef typename boost::call_traits<value_type>::param_type param_type;

	void put (param_type item)
	{
	  boost::mutex::scoped_lock lock( mutex_ );
	  container_.push_back (item);
	  not_empty_.notify_one();
	}

	value_type get ()
	{
	  value_type v;
	  get(&v);
	  return v;
	}

	void get (value_type *item)
	{
	  boost::mutex::scoped_lock lock( mutex_ );
          while (! is_not_empty())
          {
            not_empty_.wait(lock);
          }
	  *item = container_.front(); container_.pop_front();
	}

        inline
        size_type size (void) const
        {
	  boost::mutex::scoped_lock lock( mutex_ );
          return container_.size();
        }

      private:
	inline bool is_not_empty() const { return ! container_.empty(); }
	inline bool is_not_full() const { return true; }

	container_type container_;
	boost::mutex mutex_;
	boost::condition not_empty_;
      };
    }
  }
}

#endif
