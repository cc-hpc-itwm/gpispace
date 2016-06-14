#pragma once

#include <boost/utility.hpp>

#include <condition_variable>
#include <list>
#include <mutex>

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
        std::unique_lock<std::mutex> lock (m_mtx);
        m_get_cond.wait (lock, [this] { return !m_container.empty(); });

        T t (std::move (m_container.front()));
        m_container.pop_front();
        return t;
      }

      template<class... Args> void put (Args&&... args)
      {
        std::unique_lock<std::mutex> const _ (m_mtx);
        m_container.emplace_back (std::forward<Args> (args)...);
        m_get_cond.notify_one();
      }

      template<typename FwdIt> void put_many (FwdIt begin, FwdIt end)
      {
        std::unique_lock<std::mutex> const _ (m_mtx);
        m_container.insert (m_container.end(), begin, end);
        m_get_cond.notify_all();
      }

      void INDICATES_A_RACE_clear()
      {
        std::unique_lock<std::mutex> const _ (m_mtx);
        m_container.clear();
      }
    private:
      mutable std::mutex m_mtx;
      std::condition_variable m_get_cond;

      container_type m_container;
    };

    template<typename T>
      class ptr_queue : public boost::noncopyable
    {
    public:
      std::unique_ptr<T> get()
      {
        std::unique_lock<std::mutex> lock (m_mtx);
        m_get_cond.wait (lock, [this] { return !m_container.empty(); });

        std::unique_ptr<T> ret (std::move (m_container.front()));
        m_container.pop_front();
        return ret;
      }

      void put (std::unique_ptr<T> t)
      {
        std::unique_lock<std::mutex> const _ (m_mtx);
        m_container.push_back (std::move (t));
        m_get_cond.notify_one();
      }

    private:
      mutable std::mutex m_mtx;
      std::condition_variable m_get_cond;

      std::list<std::unique_ptr<T>> m_container;
    };
  }
}
