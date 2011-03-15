#include "transfer_queue.hpp"

#include <fhglog/minimal.hpp>

#include <boost/make_shared.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      transfer_queue_t::transfer_queue_t (task_queue_t *queue_to_pool)
        : m_paused (false)
        , m_blocking_tasks (*queue_to_pool)
      {
        m_thread = boost::make_shared<boost::thread>
          (boost::bind ( &transfer_queue_t::worker
                       , this
                       )
          );
      }

      transfer_queue_t::~transfer_queue_t ()
      {
        // clear all pending and all finished
        m_thread->interrupt ();
        m_thread->join ();
      }

      void
      transfer_queue_t::worker ()
      {
        for (;;)
        {
          wait_until_unpaused ();
          task_ptr task = m_task_queue.pop();
          // readDMA, writeDMA, waitOnQueue
          task->execute ();
        }
      }

      void
      transfer_queue_t::enqueue (const memory_transfer_t &t)
      {
// TODO: it is not sufficient to only check for local/nonlocal
//       we need to check for shm/gpi as well
//       i.e. the src-area should implement a "create_transfer_task(dst)"
//       function that implements the transfer details
        const bool src_is_local
          (t.src_area->is_local(gpi::pc::type::memory_region_t( t.src_location
                                                              , t.amount
                                                              )
                               )
          );
        const bool dst_is_local
          (t.dst_area->is_local(gpi::pc::type::memory_region_t( t.dst_location
                                                              , t.amount
                                                              )
                               )
          );

        if (src_is_local && dst_is_local)
        {
          LOG(TRACE, "transfering data locally (memcpy)");
          task_ptr task (boost::make_shared<task_t>
                        ("memcpy", boost::bind( &transfer_queue_t::do_memcpy
                                              , t
                                              )
                        )
                        );
          m_dispatched.insert (task);
          m_blocking_tasks.push (task);
        }
        else if (src_is_local && !dst_is_local)
        {
          LOG(TRACE, "transfering local data to remote location");
          throw std::runtime_error ("writeDMA transfers not yet implemented");
        }
        else if (dst_is_local && !src_is_local)
        {
          LOG(INFO, "transfering remote data to local location");
          throw std::runtime_error ("readDMA transfers not yet implemented");
        }
        else
        {
          throw std::runtime_error
            ( "illegal memory transfer requested:"
              " source and destination cannot both be remote"
            );
        }

        // build task depending on memory transfer kind
        //    memcopy task -> send to m_blocking
        //       link the task to this queue
        //       execute:
        //           memcpy()
        //    rdma task    -> send to m_task_queue
      }

      void
      transfer_queue_t::do_memcpy (memory_transfer_t t)
      {
        LOG(TRACE, "memcpy: " << t);

        std::memcpy( t.dst_area->pointer_to(t.dst_location)
                   , t.src_area->pointer_to(t.src_location)
                   , t.amount
                   );
      }

      void
      transfer_queue_t::wait_until_unpaused () const
      {
        lock_type lock (m_mutex);
        while (is_paused())
        {
          m_resume_condition.wait (lock);
        }
      }

      void
      transfer_queue_t::resume ()
      {
        lock_type lock (m_mutex);
        m_paused = false;
        m_resume_condition.notify_all();
      }

      void
      transfer_queue_t::pause ()
      {
        lock_type lock (m_mutex);
        m_paused = true;
      }

      bool
      transfer_queue_t::is_paused () const
      {
        return m_paused;
      }

      std::size_t
      transfer_queue_t::wait ()
      {
        // 1. get current size of pending
        // 2. get current size of finished
        // 3. submit a task with the following state and behaviour
        //     state: count = sum(1.,2.)
        //     behav: --> gpiWaitOnQueue
        //            -->
        return 0;
      }
    }
  }
}
