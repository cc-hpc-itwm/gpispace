// alexander.petry@itwm.fraunhofer.de

#ifndef WE_MGMT_LAYER_SET_HPP
#define WE_MGMT_LAYER_SET_HPP 1

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
      template <typename T>
      class set : boost::noncopyable
      {
      public:
        typedef set<T> this_type;

	typedef boost::unordered_set<T> container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::value_type value_type;
	typedef typename boost::call_traits<value_type>::param_type param_type;

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

	container_type container_;
	boost::mutex mutex_;
	boost::condition not_empty_;
      };
    }
  }
}

#endif
