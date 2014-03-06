#ifndef FHG_UTIL_THREAD_QUEUE_HPP
#define FHG_UTIL_THREAD_QUEUE_HPP

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/utility.hpp>

#include <list>

namespace fhg
{
  namespace thread
  {
    template<typename T>
      class queue : public boost::noncopyable
    {
    public:
      typedef std::list<T> container_type;
      typedef typename container_type::size_type size_type;

      T get()
      {
        boost::unique_lock<boost::recursive_mutex> lock (m_mtx);
        m_get_cond.wait
          (lock, not boost::bind (&container_type::empty, &m_container));

        T t (m_container.front()); m_container.pop_front();
        return t;
      }

      void put (T const & t)
      {
        boost::unique_lock<boost::recursive_mutex> const _ (m_mtx);
        m_container.push_back (t);
        m_get_cond.notify_one();
      }

      size_type INDICATES_A_RACE_size() const
      {
        boost::unique_lock<boost::recursive_mutex> const _ (m_mtx);
        return m_container.size();
      }

      void INDICATES_A_RACE_clear()
      {
        boost::unique_lock<boost::recursive_mutex> const _ (m_mtx);
        m_container.clear();
      }
    private:
      mutable boost::recursive_mutex m_mtx;
      boost::condition_variable_any m_get_cond;

      container_type m_container;
    };

    template<typename T>
      class ptr_queue : public boost::noncopyable
    {
    public:
      typedef boost::ptr_list<T> container_type;

      typename container_type::auto_type get()
      {
        boost::unique_lock<boost::recursive_mutex> lock (m_mtx);
        m_get_cond.wait
          (lock, not boost::bind (&container_type::empty, &m_container));

        return m_container.pop_front();
      }

      void put (std::auto_ptr<T> t)
      {
        boost::unique_lock<boost::recursive_mutex> const _ (m_mtx);
        m_container.push_back (t);
        m_get_cond.notify_one();
      }

    private:
      mutable boost::recursive_mutex m_mtx;
      boost::condition_variable_any m_get_cond;

      container_type m_container;
    };
  }
}

#endif
