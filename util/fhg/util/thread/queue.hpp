#ifndef FHG_UTIL_THREAD_QUEUE_HPP
#define FHG_UTIL_THREAD_QUEUE_HPP

#include <boost/bind.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/utility.hpp>

namespace fhg
{
  namespace thread
  {
    template < typename T
             , template < typename
                        , typename
                        > class Container
             , typename Allocator = std::allocator<T>
             >
    class queue : public boost::noncopyable
    {
    public:
      typedef queue<T, Container, Allocator> this_type;
      typedef boost::recursive_mutex            mutex;
      typedef boost::unique_lock<mutex>     lock_type;
      typedef boost::condition_variable_any condition;

      typedef T                                  value_type;
      typedef Container<T, Allocator>            container_type;
      typedef typename container_type::size_type size_type;

      T get()
      {
        lock_type lock(m_mtx);
        m_get_cond.wait ( lock
                        , boost::bind ( &this_type::is_element_available
                                      , this
                                      )
                        );
        return _get_impl ();
      }

      void put (T const & t)
      {
        lock_type lock(m_mtx);
        m_put_cond.wait ( lock
                        , boost::bind ( &this_type::is_free_slot_available
                                      , this
                                      )
                        );
        _put_impl (t);
      }

      size_type size() const
      {
        lock_type lock(m_mtx);
        return m_container.size();
      }

      bool empty() const
      {
    	  lock_type lock(m_mtx);
    	  return m_container.empty();
      }

      template <typename Pred>
      size_t remove_if (Pred pred)
      {
        lock_type lock(m_mtx);
        size_t cnt (0);
        for ( typename container_type::iterator it (m_container.begin())
            ; it != m_container.end()
            ;
            )
        {
          if (pred(*it))
          {
            it = m_container.erase(it);
            ++cnt;
          }
          else
          {
            ++it;
          }
        }

        if (cnt)
          m_put_cond.notify_one ();

        return cnt;
      }

      void clear ()
      {
        lock_type lock(m_mtx);
        while (not m_container.empty())
          m_container.pop_front();
        m_put_cond.notify_one ();
      }
    private:
      T _get_impl ()
      {
        T t (m_container.front()); m_container.pop_front();
        m_put_cond.notify_one ();
        return t;
      }

      void _put_impl (T const & t)
      {
        m_container.push_back(t);
        m_get_cond.notify_one();
      }

      bool is_element_available () const
      {
        return not m_container.empty ();
      }

      bool is_free_slot_available () const
      {
        if (m_buffer_size)
          return m_container.size () < m_buffer_size;
        else
          return true;
      }

      mutable mutex     m_mtx;
      mutable condition m_put_cond;
      mutable condition m_get_cond;

      size_type      m_buffer_size;
      container_type m_container;
    };
  }
}

#endif
