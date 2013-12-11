#ifndef FHG_UTIL_THREAD_QUEUE_HPP
#define FHG_UTIL_THREAD_QUEUE_HPP

#include <boost/bind.hpp>
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
        boost::unique_lock<boost::recursive_mutex> lock(m_mtx);
        m_get_cond.wait ( lock
                        , boost::bind ( &queue<T>::is_element_available
                                      , this
                                      )
                        );

        T t (m_container.front()); m_container.pop_front();
        return t;
      }

      void put (T const & t)
      {
        boost::unique_lock<boost::recursive_mutex> const _ (m_mtx);
        m_container.push_back(t);
        m_get_cond.notify_one();
      }

      size_type size() const
      {
        boost::unique_lock<boost::recursive_mutex> const _ (m_mtx);
        return m_container.size();
      }

      bool empty() const
      {
        boost::unique_lock<boost::recursive_mutex> const _ (m_mtx);
        return m_container.empty();
      }

      template <typename Pred>
        size_t remove_if (Pred pred)
      {
        boost::unique_lock<boost::recursive_mutex> const _ (m_mtx);
        size_t cnt (0);
        for ( typename container_type::iterator it (m_container.begin())
            ; it != m_container.end()
            ;
            )
        {
          if (pred (*it))
          {
            it = m_container.erase(it);
            ++cnt;
          }
          else
          {
            ++it;
          }
        }

        return cnt;
      }

      void clear()
      {
        boost::unique_lock<boost::recursive_mutex> const _ (m_mtx);
        m_container.clear();
      }
    private:
      bool is_element_available() const
      {
        return not m_container.empty ();
      }

      mutable boost::recursive_mutex m_mtx;
      boost::condition_variable_any m_get_cond;

      container_type m_container;
    };
  }
}

#endif
