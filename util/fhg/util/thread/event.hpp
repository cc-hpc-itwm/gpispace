#ifndef FHG_UTIL_EVENT_HPP
#define FHG_UTIL_EVENT_HPP 1

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
        bool                      m_signalled;
        mutable mutex_type        m_mutex;
        mutable condition_type    m_condition;
      public:
        typedef T value_type;

        explicit
        event ()
          : m_event ()
          , m_signalled (false)
        {}

        template <typename U>
        explicit
        event (U const & u)
          : m_event (u)
          , m_signalled (false)
        {}

        template <typename U>
        void wait(U & u)
        {
          lock_type lock(m_mutex);

          while (! m_signalled)
          {
            m_condition.wait(lock);
          }

          u = m_event;
          m_signalled = false;
        }

        template <typename U>
        bool timed_wait(U & u, boost::system_time const& abs_time)
        {
          lock_type lock(m_mutex);
          while (! m_signalled)
          {
            if (! m_condition.timed_wait(lock, abs_time))
            {
              return false;
            }
          }
          u = m_event;
          m_signalled = false;
          return true;
        }

        void notify(value_type const & u)
        {
          lock_type lock(m_mutex);
          m_event = u;
          m_signalled = true;
          m_condition.notify_one();
        }
      };

      template<>
        class event<void> : boost::noncopyable
      {
        bool _signalled;
        mutable boost::mutex _mutex;
        mutable boost::condition_variable _condition;
      public:

        event()
          : _signalled (false)
        {}

        void wait()
        {
          boost::mutex::scoped_lock lock (_mutex);

          while (!_signalled)
          {
            _condition.wait(lock);
          }

          _signalled = false;
        }

        bool timed_wait (boost::system_time const& abs_time)
        {
          boost::mutex::scoped_lock lock (_mutex);
          while (!_signalled)
          {
            if (!_condition.timed_wait (lock, abs_time))
            {
              return false;
            }
          }
          _signalled = false;
          return true;
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
