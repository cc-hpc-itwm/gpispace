#include <gpi-space/pc/memory/transfer_manager.hpp>

#include <fhglog/log_to_GLOBAL_logger.hpp>

#include <fhg/util/make_unique.hpp>

#include <boost/make_shared.hpp>

#include <gpi-space/gpi/api.hpp>

#include <functional>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      transfer_manager_t::transfer_manager_t (api::gpi_api_t& gpi_api)
        : _gpi_api (gpi_api)
      {
        const std::size_t number_of_queues (gpi_api.number_of_queues());
        for (std::size_t i(0); i < number_of_queues; ++i)
        {
          m_queues.push_back
            (boost::make_shared<transfer_queue_t>());
        }

        for (size_t i = 0; i < number_of_queues; ++i)
        {
          constexpr size_t DEF_BUFFER_SIZE (4194304);

          m_memory_buffer_pool.put
            (fhg::util::make_unique<buffer_t> (DEF_BUFFER_SIZE));
        }
      }

      void
      transfer_manager_t::transfer (memory_transfer_t const &t)
      {
        if (t.queue >= m_queues.size())
        {
          LOG( ERROR
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
          LOG( ERROR
             , "cannot wait on queue: no such queue: " << queue
             );
          throw std::invalid_argument ("no such queue");
        }
        else
        {
          try
          {
            task_ptr wtask (boost::make_shared<task_t>
                           ("wait_on_queue", std::bind( &api::gpi_api_t::wait_dma
                                                      , &_gpi_api
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
            LOG ( ERROR
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
