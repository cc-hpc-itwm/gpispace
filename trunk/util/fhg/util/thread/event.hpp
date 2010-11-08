#ifndef FHG_UTIL_EVENT_HPP
#define FHG_UTIL_EVENT_HPP 1

#include <fhg/util/thread/semaphore.hpp>

namespace fhg
{
  namespace util
  {
    namespace thread
    {
      template <typename T>
      class event : boost::noncopyable
      {
        T                         m_event;
        semaphore                 m_signal;
        semaphore                 m_semaphore;
      public:
        typedef T value_type;

        explicit
        event ()
          : m_event ()
          , m_signal (0)
          , m_semaphore(1)
        {}

        template <typename U>
        explicit
        event (U const & u)
          : m_event (u)
          , m_signal (0)
          , m_semaphore(1)
        {}

        template <typename U>
        void wait(U & u)
        {
          m_signal.P();

          m_semaphore.P();
          u = m_event;
          m_semaphore.V();
        }

        void notify(value_type const & u)
        {
          m_semaphore.P();
          m_event = u;
          m_semaphore.V();

          m_signal.V();
        }

        /*
        void notify()
        {
          notify (T());
        }
        */

        std::size_t count () const
        {
          return m_signal.count();
        }
      };
    }
  }
}

#endif
