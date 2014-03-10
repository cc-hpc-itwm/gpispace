#include <gpi-space/pc/memory/transfer_queue.hpp>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      transfer_queue_t::transfer_queue_t()
        : m_enabled (true)
        , m_thread (boost::bind (&transfer_queue_t::worker, this))
      {}

      void
      transfer_queue_t::worker ()
      {
        for (;;)
        {
          m_task_queue.get()->execute();
        }
      }

      void
      transfer_queue_t::enqueue (const task_ptr &task)
      {
        task_list_t tasks;
        tasks.push_back (task);
        enqueue (tasks);
      }

      void
      transfer_queue_t::enqueue (const task_list_t & tasks)
      {
        if (is_disabled ())
        {
          throw std::runtime_error
            ("queue permanently disabled due to previous errors");
        }

        BOOST_FOREACH(task_ptr const task, tasks)
        {
            m_task_queue.put (task);
        }

        // this  needs to  be atomic,  otherwise (enqueue();  wait();)  would be
        // broken
        {
          boost::mutex::scoped_lock const _ (_mutex_dispatched);
          m_dispatched.insert (tasks.begin(), tasks.end());
        }
      }

      void
      transfer_queue_t::disable ()
      {
        boost::mutex::scoped_lock const _ (m_mutex);
        m_enabled = false;
      }

      bool
      transfer_queue_t::is_disabled () const
      {
        return not m_enabled;
      }

      std::size_t
      transfer_queue_t::wait ()
      {
        task_set_t wait_on_tasks;
        {
          boost::mutex::scoped_lock const _ (_mutex_dispatched);
          m_dispatched.swap (wait_on_tasks);
        }

        std::size_t const res (wait_on_tasks.size());
        while (! wait_on_tasks.empty())
        {
          task_ptr task(*wait_on_tasks.begin());
          wait_on_tasks.erase (wait_on_tasks.begin());
          task->wait();

          // TODO: WORK HERE:  this failure should be propagated  to the correct
          // process container
          if (task->has_failed())
          {
            throw std::runtime_error
              ( "task failed: " + task->get_name ()
              + ": " + task->get_error_message ()
              );
          }
        }
        return res;
      }
    }
  }
}
