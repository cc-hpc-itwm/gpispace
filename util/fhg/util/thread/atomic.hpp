#ifndef FHG_UTIL_THREAD_ATOMIC
#define FHG_UTIL_THREAD_ATOMIC

#ifdef HAS_STD_ATOMIC
#  include <atomic>
#else
#  include <boost/thread/mutex.hpp>
#  include <boost/thread/locks.hpp>
#endif

namespace fhg
{
  namespace thread
  {
#ifdef HAS_STD_ATOMIC
    using std::atomic;
#else
    template <typename value_type>
    class atomic
    {
      typedef boost::lock_guard<boost::mutex> lock_type;
    public:
      atomic ()
        : m_value ()
      {}

      explicit
      atomic (value_type v)
        : m_value (v)
      {}

      operator value_type () const
      {
        lock_type lock (m_mutex);
        return m_value;
      }

      atomic<value_type> & operator++ ()
      {
        lock_type lock (m_mutex);
        ++m_value;
        return *this;
      }

      atomic<value_type> operator++ (int)
      {
        lock_type lock (m_mutex);
        value_type old (m_value);
        ++m_value;
        return atomic<value_type> (old);
      }

      template <typename U>
      atomic<value_type> & operator+= (U u)
      {
        lock_type lock (m_mutex);
        m_value += u;
        return *this;
      }

      template <typename U>
      atomic<value_type> & operator-= (U u)
      {
        lock_type lock (m_mutex);
        m_value -= u;
        return *this;
      }

      template <typename U>
      atomic<value_type> & operator= (U u)
      {
        lock_type lock (m_mutex);
        m_value = u;
        return *this;
      }

      atomic<value_type> & operator-- ()
      {
        lock_type lock (m_mutex);
        --m_value;
        return *this;
      }

      atomic<value_type> operator-- (int)
      {
        lock_type lock (m_mutex);
        value_type old (m_value);
        --m_value;
        return atomic<value_type> (old);
      }
    private:
      //! \note Can't use boost::noncopyable, as there is an operator=<T>.
      atomic (const atomic &);
      atomic & operator=(const atomic &);

      mutable boost::mutex m_mutex;
      value_type m_value;
    };
#endif
  }
}

#endif
