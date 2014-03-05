// alexander.petry@itwm.fraunhofer.de

#ifndef SDPA_DAEMON_SYNCHRONIZED_QUEUE_HPP
#define SDPA_DAEMON_SYNCHRONIZED_QUEUE_HPP

#include <boost/thread.hpp>

#include <stdexcept>

namespace sdpa
{
  namespace daemon
  {
    template <class Container> class SynchronizedQueue
    {
    public:
      typedef typename Container::value_type value_type;

      inline value_type pop()
      {
        boost::mutex::scoped_lock const _ (mtx_);
        if (container_.empty())
        {
          throw std::runtime_error ("queue is empty");
        }

        value_type item = container_.front();
        container_.pop_front();
        return item;
      }

      inline void push (value_type item)
      {
        boost::mutex::scoped_lock const _ (mtx_);
        container_.push_back (item);
        not_empty_.notify_one();
      }

      inline void push_front (value_type item)
      {
        boost::mutex::scoped_lock const _ (mtx_);
        container_.push_front (item);
        not_empty_.notify_one();
      }

      inline bool empty() const
      {
        boost::mutex::scoped_lock const _ (mtx_);
        return container_.empty();
      }

      inline size_t erase (const value_type& item)
      {
        boost::mutex::scoped_lock const _ (mtx_);
        size_t count (0);
        typename Container::iterator iter (container_.begin());
        while (iter != container_.end())
        {
          if (item == *iter)
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
      mutable boost::mutex mtx_;
      boost::condition_variable_any not_empty_;
      Container container_;
    };
  }
}
#endif
