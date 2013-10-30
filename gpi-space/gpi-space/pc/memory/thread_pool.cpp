#include "thread_pool.hpp"
#include <boost/foreach.hpp>

namespace gpi
{
  namespace pc
  {
    thread_pool_t::~thread_pool_t()
    {
      clear ();
    }

    void
    thread_pool_t::add_thread(boost::thread* thrd)
    {
      lock_type lock(m_mutex);
      m_pool.insert (thrd);
    }

    void
    thread_pool_t::interrupt()
    {
      lock_type lock(m_mutex);
      BOOST_FOREACH(boost::thread* t, m_pool)
      {
        t->interrupt();
      }
    }

    void
    thread_pool_t::join()
    {
      lock_type lock(m_mutex);
      BOOST_FOREACH(boost::thread* t, m_pool)
      {
        t->join();
      }
    }

    void
    thread_pool_t::clear()
    {
      std::set<boost::thread*> threads;
      {
        lock_type lock(m_mutex);
        m_pool.swap (threads);
      }

      while (!threads.empty())
      {
        boost::thread *t(*threads.begin());
        threads.erase(threads.begin());

        if (t->joinable())
        {
          t->interrupt();
          t->join();
        }
        delete t;
      }
    }
  }
}
