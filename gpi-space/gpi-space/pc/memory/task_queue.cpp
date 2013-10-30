#include "task_queue.hpp"

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      task_queue_t::~task_queue_t ()
      {
        clear();
      }

      void
      task_queue_t::clear ()
      {
        lock_type lock(m_mutex);
        m_tasks.clear ();
      }

      task_queue_t::task_ptr
      task_queue_t::pop()
      {
        lock_type lock(m_mutex);
        while (m_tasks.empty())
        {
          m_task_available.wait(lock);
        }

        task_ptr t(m_tasks.front());
        m_tasks.pop_front();

        if (! m_tasks.empty())
          m_task_available.notify_one();

        return t;
      }

      void
      task_queue_t::push(task_ptr t)
      {
        lock_type lock(m_mutex);
        m_tasks.push_back (t);
        m_task_available.notify_one();
      }

      void
      task_queue_t::cancel()
      {
        lock_type lock(m_mutex);
        for ( container_type::iterator t_it (m_tasks.begin())
            ; t_it != m_tasks.end()
            ; ++t_it
            )
        {
          (*t_it)->cancel();
        }
      }

      void
      task_queue_t::cancel(task_ptr t)
      {
        lock_type lock(m_mutex);
        for ( container_type::iterator t_it (m_tasks.begin())
            ; t_it != m_tasks.end()
            ; ++t_it
            )
        {
          if (*t_it == t)
          {
            t->cancel();
            break;
          }
        }
      }

      bool
      task_queue_t::empty() const
      {
        lock_type lock(m_mutex);
        return m_tasks.empty();
      }

      task_queue_t::size_type
      task_queue_t::size () const
      {
        lock_type lock(m_mutex);
        return m_tasks.size();
      }
    }
  }
}
