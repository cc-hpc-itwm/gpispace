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

namespace we { namespace mgmt { namespace detail {
  template <typename T>
  class queue
  {
  public:
	typedef boost::circular_buffer<T> container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::value_type value_type;
	typedef typename boost::call_traits<value_type>::param_type param_type;

	explicit queue(size_type capacity) : unread_(0), capacity_(capacity), container_(capacity) {}

	void put (param_type item)
	{
	  boost::mutex::scoped_lock lock( mutex_ );
	  not_full_.wait (lock, boost::bind(&queue<value_type>::is_not_full, this));
	  container_.push_front (item);
	  ++unread_;
	  lock.unlock();
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
	  not_empty_.wait (lock, boost::bind(&queue<value_type>::is_not_empty, this));
	  *item = container_[--unread_];
	  lock.unlock();
	  not_full_.notify_one();
	}

  private:
	queue(queue const &);
	queue & operator=(queue const &);

	inline bool is_not_empty() const { return unread_ > 0; }
	inline bool is_not_full() const { return unread_ < capacity_; /* container_.capacity(); */ }

	size_type unread_;
	const size_type capacity_;
	container_type container_;
	boost::mutex mutex_;
	boost::condition not_empty_;
	boost::condition not_full_;
  };
}}}
