#pragma once

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
        m_get_cond.wait (lock, [this] { return !m_container.empty(); });

        T t (std::move (m_container.front()));
        m_container.pop_front();
        return t;
      }

      template<class... Args> void put (Args&&... args)
      {
        boost::unique_lock<boost::recursive_mutex> const _ (m_mtx);
        m_container.emplace_back (std::forward<Args> (args)...);
        m_get_cond.notify_one();
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
      std::unique_ptr<T> get()
      {
        boost::unique_lock<boost::recursive_mutex> lock (m_mtx);
        m_get_cond.wait (lock, [this] { return !m_container.empty(); });

        std::unique_ptr<T> ret (std::move (m_container.front()));
        m_container.pop_front();
        return ret;
      }

      void put (std::unique_ptr<T> t)
      {
        boost::unique_lock<boost::recursive_mutex> const _ (m_mtx);
        m_container.push_back (std::move (t));
        m_get_cond.notify_one();
      }

    private:
      mutable boost::recursive_mutex m_mtx;
      boost::condition_variable_any m_get_cond;

      std::list<std::unique_ptr<T>> m_container;
    };
  }
}
