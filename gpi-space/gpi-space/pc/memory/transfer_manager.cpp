#include "transfer_manager.hpp"

#include <fhglog/minimal.hpp>

#include <boost/make_shared.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      transfer_manager_t::transfer_manager_t ( const std::size_t number_of_queues
                                             , const std::size_t memcpy_pool_size
                                             )
      {
        for (std::size_t i(0); i < number_of_queues; ++i)
        {
          m_queues.push_back
            (boost::make_shared<transfer_queue_t>(i, &m_worker_queue));
        }
        m_worker_pool.add
          ( boost::bind(&transfer_manager_t::worker, this)
          , std::max(std::size_t(1), memcpy_pool_size)
          );
      }

      transfer_manager_t::~transfer_manager_t()
      {}

      void
      transfer_manager_t::transfer (memory_transfer_t const &t)
      {
        if (t.queue >= m_queues.size())
        {
          CLOG( ERROR
              , "gpi.memory"
              , "cannot enqueue request: no such queue: " << t.queue
              );
          throw std::invalid_argument ("no such queue");
        }
        m_queues[t.queue]->enqueue (t);
      }

      std::size_t
      transfer_manager_t::wait_on_queue (const std::size_t queue)
      {
        if (queue >= m_queues.size())
        {
          CLOG( ERROR
              , "gpi.memory"
              , "cannot wait on queue: no such queue: " << queue
              );
          throw std::invalid_argument ("no such queue");
        }
        return m_queues[queue]->wait ();
      }

      void
      transfer_manager_t::worker()
      {
        for (;;)
        {
          task_ptr task (m_worker_queue.pop());
          task->execute();
          if (task->has_failed())
          {
            LOG( ERROR
               , "task failed: " << task->get_name() << ": "
               << task->get_error_message()
               );
          }
          else if (task->has_finished())
          {
            DLOG(TRACE, "transfer done: " << task->get_name());
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
    }
  }
}
