#include "transfer_manager.hpp"

#include <fhglog/minimal.hpp>

#include <boost/make_shared.hpp>

#include <gpi-space/gpi/api.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      namespace helper
      {
        static
        void do_wait_on_queue (const std::size_t q)
        {
          DLOG(TRACE, "gpi::wait_dma(" << q << ")");

#ifndef NDEBUG
          std::size_t s(
#endif
                        gpi::api::gpi_api_t::get().wait_dma (q)
#ifndef NDEBUG
                       )
#endif
            ;

          DLOG(TRACE, "gpi::wait_dma(" << q << ") = " << s);
        }
      }

      void
      transfer_manager_t::start ( const std::size_t number_of_queues
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

        for (size_t i = 0; i < number_of_queues; ++i)
        {
          m_memory_buffer_pool.add (new buffer_t (DEF_BUFFER_SIZE));
        }
      }

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

        task_list_t task_list;

        t.src_area->get_transfer_tasks ( t.src_location
                                       , t.dst_location
                                       , *t.dst_area
                                       , t.amount
                                       , t.queue
                                       , m_memory_buffer_pool
                                       , task_list
                                       );

        m_queues [t.queue]->enqueue (task_list);
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
        else
        {
          try
          {
            task_ptr wtask (boost::make_shared<task_t>
                           ("wait_on_queue", boost::bind( &helper::do_wait_on_queue
                                                        , queue
                                                        )
                           )
                           );
            m_queues [queue]->enqueue (wtask);
            wtask->wait ();

            if (wtask->has_failed ())
            {
              throw std::runtime_error
                ("wait failed: " + wtask->get_error_message ());
            }

            // account for the wait task  before, but since another thread could
            // already  have consumed  his and  our  wait task,  we just  always
            // decrement  by  one  if  we  got  more  than  1  finished  task  -
            // specification just  means, ok the  tasks have finished,  but it's
            // not specific about the exact number to return - quite ugly

            const size_t tasks_finished (m_queues[queue]->wait ());
            if (tasks_finished)
              return tasks_finished - 1;
            else
              return tasks_finished;
          }
          catch (std::exception const & ex)
          {
            MLOG ( ERROR
                 , "marking queue " << queue << " permanently as failed!"
                 );
            m_queues [queue]->disable ();

            throw;
          }
        }
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
