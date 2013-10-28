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
#include <boost/utility.hpp>

namespace we {
  namespace mgmt {
    namespace detail {

      template <typename T, unsigned long long CAPACITY>
      class set : boost::noncopyable
      {
      public:
        typedef set<T, CAPACITY> this_type;

	typedef boost::unordered_set<T> container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::value_type value_type;
	typedef typename boost::call_traits<value_type>::param_type param_type;

        set()
        {}

	void put (param_type item)
	{
	  boost::mutex::scoped_lock lock( mutex_ );
          while (! is_not_full())
          {
            not_full_.wait (lock);
          }
	  container_.insert (item);
	  not_empty_.notify_one();
	}

	value_type get ()
	{
	  value_type v;
	  get(v);
	  return v;
	}

	void get (value_type & item)
	{
	  boost::mutex::scoped_lock lock( mutex_ );
          while (! is_not_empty())
          {
            not_empty_.wait (lock);
          }
          typename container_type::iterator i (container_.begin());
          item = *i;
          container_.erase (i);
	  not_full_.notify_one();
	}

        inline
        size_type size (void) const
        {
	  boost::mutex::scoped_lock lock( mutex_ );
          return container_.size();
        }

        void erase (const value_type & item)
        {
	  boost::mutex::scoped_lock lock( mutex_ );
          container_.erase (item);
	  not_full_.notify_one();
        }

      private:
	inline bool is_not_empty() const { return container_.size() > 0; }
	inline bool is_not_full() const { return container_.size() < CAPACITY; }

	container_type container_;
	boost::mutex mutex_;
	boost::condition not_empty_;
	boost::condition not_full_;
      };

      template <typename T>
      class set<T, 0> : boost::noncopyable
      {
      public:
        typedef set<T, 0> this_type;

	typedef boost::unordered_set<T> container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::value_type value_type;
	typedef typename boost::call_traits<value_type>::param_type param_type;

        set()
        {}

	void put (param_type item)
	{
	  boost::mutex::scoped_lock lock( mutex_ );
	  container_.insert (item);
	  not_empty_.notify_one();
	}

	value_type get ()
	{
	  value_type v;
	  get(v);
	  return v;
	}

	void get (value_type & item)
	{
	  boost::mutex::scoped_lock lock( mutex_ );
          while (! is_not_empty())
          {
            not_empty_.wait (lock);
          }
          typename container_type::iterator i (container_.begin());
          item = *i;
          container_.erase (i);
	}

        inline
        size_type size (void) const
        {
          return container_.size();
        }

        void erase (const value_type & item)
        {
	  boost::mutex::scoped_lock lock( mutex_ );
          container_.erase (item);
        }

      private:
	inline bool is_not_empty() const { return container_.size() > 0; }
	inline bool is_not_full() const { return true; }

	container_type container_;
	boost::mutex mutex_;
	boost::condition not_empty_;
      };
    }
  }
}

#endif
