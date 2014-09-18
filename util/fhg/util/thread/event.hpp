#ifndef FHG_UTIL_THREAD_EVENT_HPP
#define FHG_UTIL_THREAD_EVENT_HPP

#include <boost/thread.hpp>

namespace fhg
{
  namespace util
  {
    namespace thread
    {
      template <typename T = void> class event;

      template <typename T>
        class event : boost::noncopyable
      {
        typedef boost::mutex mutex_type;
        typedef boost::condition_variable condition_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        T                         m_event;
        bool                      m_signalled {false};
        mutable mutex_type        m_mutex;
        mutable condition_type    m_condition;
      public:

        T wait()
        {
          lock_type lock(m_mutex);

          while (! m_signalled)
          {
            m_condition.wait(lock);
          }

          m_signalled = false;
          return std::move (m_event);
        }

        void notify (T u)
        {
          lock_type lock(m_mutex);
          m_event = std::move (u);
          m_signalled = true;
          m_condition.notify_one();
        }
      };

      template<>
        class event<void> : boost::noncopyable
      {
        bool _signalled {false};
        mutable boost::mutex _mutex;
        mutable boost::condition_variable _condition;
      public:
        void wait()
        {
          boost::mutex::scoped_lock lock (_mutex);

          while (!_signalled)
          {
            _condition.wait(lock);
          }

          _signalled = false;
        }

        void notify()
        {
          boost::mutex::scoped_lock const lock (_mutex);
          _signalled = true;
          _condition.notify_one();
        }
      };
    }
  }
}

#endif
