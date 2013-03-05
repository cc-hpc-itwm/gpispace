#ifndef FHG_UTIL_THREAD_QUEUE_HPP
#define FHG_UTIL_THREAD_QUEUE_HPP

#include <boost/bind.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <fhg/util/thread/selectable.hpp>

namespace fhg
{
  namespace thread
  {
    struct operation_timedout : std::runtime_error
    {
      operation_timedout (std::string const &op)
        : std::runtime_error ("'" + op + "' timedout")
      {}

      ~operation_timedout () throw() {}
    };

    template < typename T
             , template < typename
                        , typename
                        > class Container
             , typename Allocator = std::allocator<T>
             >
    class queue : public virtual selectable
    {
    public:
      typedef queue<T, Container, Allocator> this_type;
      typedef boost::recursive_mutex            mutex;
      typedef boost::unique_lock<mutex>     lock_type;
      typedef boost::condition_variable_any condition;

      typedef T                                  value_type;
      typedef Container<T, Allocator>            container_type;
      typedef typename container_type::size_type size_type;

      explicit queue (size_type buffer_size = 0)
        : m_buffer_size (buffer_size)
      {}

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

      T get (boost::posix_time::time_duration duration)
      {
        boost::system_time const timeout = boost::get_system_time() + duration;
        lock_type lock (m_mtx);
        if (m_get_cond.timed_wait ( lock
                                  , timeout
                                  , boost::bind ( &this_type::is_element_available
                                                , this
                                                )
                                  )
           )
        {
          return _get_impl ();
        }
        else
        {
          throw operation_timedout ("get");
        }
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

      this_type & operator << (T const & t)
      {
        put (t);
        return *this;
      }

      void put (const T & t, boost::posix_time::time_duration duration)
      {
        boost::system_time const timeout = boost::get_system_time() + duration;
        lock_type lock (m_mtx);
        if (m_put_cond.timed_wait ( lock
                                  , timeout
                                  , boost::bind ( &this_type::is_free_slot_available
                                                , this
                                                )
                                  )
           )
        {
          return _put_impl (t);
        }
        else
        {
          throw operation_timedout ("put");
        }
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

      size_t erase (T const &t)
      {
        lock_type lock(m_mtx);
        size_t cnt (0);
        for ( typename container_type::iterator it (m_container.begin())
            ; it != m_container.end()
            ;
            )
        {
          if (*it == t) { it = m_container.erase(it); ++cnt; }
          else          ++it;
        }

        return cnt;
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

      bool is_ready_for (select_mode_t mode) const
      {
        lock_type lock(m_mtx);
        switch (mode)
        {
        case POLL_IN:
          return is_element_available ();
        case POLL_OUT:
          return is_free_slot_available ();
        default:
          throw std::runtime_error
            ("invalid argument to fhg::thread::queue::poll");
        }
      }

      // expose mutex
      mutex & get_mutex () { return m_mtx; }
    private:
      queue (queue const &);
      queue & operator= (queue const &);

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
