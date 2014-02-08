#include "transfer_manager.hpp"

#include <fhglog/LogMacros.hpp>

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
                        gpi::api::gpi_api_t::get().wait_dma (q);
        }
      }

      void
      transfer_manager_t::start (const std::size_t number_of_queues)
      {
        for (std::size_t i(0); i < number_of_queues; ++i)
        {
          m_queues.push_back
            (boost::make_shared<transfer_queue_t>(i));
        }

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
    }
  }
}
