#ifndef FHG_UTIL_THREAD_QUEUE_HPP
#define FHG_UTIL_THREAD_QUEUE_HPP

#include <boost/bind.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition_variable.hpp>

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
    class queue
    {
    public:
      typedef queue<T, Container, Allocator> this_type;
      typedef boost::recursive_mutex            mutex;
      typedef boost::unique_lock<mutex>     lock_type;
      typedef boost::condition_variable_any condition;

      typedef T value_type;
      typedef Container<T, Allocator> container_type;
      typedef typename container_type::size_type size_type;

      T get()
      {
        lock_type lock(m_mtx);
        while (m_container.empty()) m_cond.wait(lock);
        T t (m_container.front()); m_container.pop_front();
        return t;
      }

      T get (boost::posix_time::time_duration duration)
      {
        boost::system_time const timeout = boost::get_system_time() + duration;
        lock_type lock (m_mtx);
        if (m_cond.timed_wait ( lock
                              , timeout
                              , boost::bind ( &this_type::is_element_available
                                            , this
                                            )
                              )
           )
        {
          T t (m_container.front ()); m_container.pop_front ();
          return t;
        }
        else
        {
          throw operation_timedout ("get");
        }
      }

      void put(T const & t)
      {
        lock_type lock(m_mtx);
        m_container.push_back(t);
        m_cond.notify_one();
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

        return cnt;
      }

      void clear ()
      {
        lock_type lock(m_mtx);
        while (not m_container.empty())
          m_container.pop_front();
      }

      // expose mutex
      mutex & get_mutex () { return m_mtx; }
    private:
      bool is_element_available () const
      {
        return not m_container.empty ();
      }

      mutable mutex m_mtx;
      mutable condition m_cond;

      container_type m_container;
    };
  }
}

#endif
