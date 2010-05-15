#ifndef WE_MGMT_LAYER_SET_HPP
#define WE_MGMT_LAYER_SET_HPP 1

/*
 * =====================================================================================
 *
 *       Filename:  set.hpp
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

#include <boost/unordered_set.hpp>
#include <boost/call_traits.hpp>
#include <boost/progress.hpp>
#include <boost/bind.hpp>

namespace we { namespace mgmt { namespace detail {
  template <typename T>
  class set
  {
  public:
    typedef set<T> this_type;

	typedef boost::unordered_set<T> container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::value_type value_type;
	typedef typename boost::call_traits<value_type>::param_type param_type;

	explicit set(const size_type capacity) : capacity_(capacity), container_(capacity) {}

	void put (param_type item)
	{
	  boost::mutex::scoped_lock lock( mutex_ );
	  not_full_.wait (lock, boost::bind(&this_type::is_not_full, this));
	  container_.insert (item);
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
	  not_empty_.wait (lock, boost::bind(&this_type::is_not_empty, this));
          typename container_type::iterator i (container_.begin());
          *item = *i;
          container_.erase (i);
	  lock.unlock();
	  not_full_.notify_one();
	}

    inline
    size_type size (void) const
    {
      return container_.size();
    }

  private:
	set(set const &);
	set & operator=(set const &);

	inline bool is_not_empty() const { return container_.size() > 0; }
	inline bool is_not_full() const { return container_.size() < capacity_; }

	const size_type capacity_;
	container_type container_;
	boost::mutex mutex_;
	boost::condition not_empty_;
	boost::condition not_full_;
  };
}}}

#endif
