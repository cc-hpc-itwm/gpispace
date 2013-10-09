#ifndef FHG_UTIL_SEMAPHORE_HPP
#define FHG_UTIL_SEMAPHORE_HPP 1

#include <boost/thread.hpp>

namespace fhg
{
  namespace thread
  {
    class semaphore : boost::noncopyable
    {
      std::size_t               m_count;
      boost::mutex              m_mutex;
      boost::condition_variable m_available;
    public:
      explicit
      semaphore (const std::size_t cnt)
        : m_count(cnt)
      {}

      void P()
      {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        while (0 == m_count)
        {
          m_available.wait (lock);
        }
        --m_count;
      }

      void V()
      {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        ++m_count;
        m_available.notify_one();
      }

      std::size_t count () const
      {
        return m_count;
      }
    };
  }
}

#endif
