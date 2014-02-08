#include "transfer_queue.hpp"

#include <fhglog/LogMacros.hpp>

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

      transfer_queue_t::~transfer_queue_t ()
      {
        // clear all pending and all finished
        m_thread.interrupt ();
        if (m_thread.joinable())
        {
          m_thread.join ();
        }
      }

      void
      transfer_queue_t::worker ()
      {
        for (;;)
        {
          task_ptr task = m_task_queue.pop();
          task->execute ();
          if (task->has_failed())
          {
            LOG( ERROR
               , "task failed: " << task->get_name() << ": "
               << task->get_error_message()
               );
          }
          else
          {
            LOG( ERROR
               , "*** STRANGE: task neither finished, nor failed, but did return?"
               << " task := " << task->get_name()
               );
          }
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

        BOOST_FOREACH(task_ptr task, tasks)
        {
            m_task_queue.push(task);
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

        std::size_t res (wait_on_tasks.size());
        while (! wait_on_tasks.empty())
        {
          task_ptr task(*wait_on_tasks.begin());
          wait_on_tasks.erase (wait_on_tasks.begin());
          task->wait();

          // TODO: WORK HERE:  this failure should be propagated  to the correct
          // process container
          if (task->has_failed())
          {
            LOG( ERROR
               , "transfer " << task->get_name()
               << " failed: " << task->get_error_message()
               );

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
