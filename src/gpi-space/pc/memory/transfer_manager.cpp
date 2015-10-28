#include <gpi-space/pc/memory/transfer_manager.hpp>

#include <fhglog/LogMacros.hpp>

#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/finally.hpp>

#include <boost/make_shared.hpp>

#include <gpi-space/gpi/api.hpp>

#include <functional>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      transfer_manager_t::transfer_manager_t ( fhg::log::Logger& logger
                                             , api::gpi_api_t& gpi_api
                                             )
        : _logger (logger)
        , _gpi_api (gpi_api)
        , _next_memcpy_id (0)
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
            (fhg::util::cxx14::make_unique<buffer_t> (DEF_BUFFER_SIZE));
        }
      }

      type::memcpy_id_t
      transfer_manager_t::transfer (memory_transfer_t const &t)
      {
        if (t.queue >= m_queues.size())
        {
          throw std::invalid_argument ("no such queue");
        }

        auto task
          ( t.src_area->get_transfer_task ( t.src_location
                                          , t.dst_location
                                          , *t.dst_area
                                          , t.amount
                                          , t.queue
                                          , m_memory_buffer_pool
                                          )
          );

        type::memcpy_id_t const memcpy_id (_next_memcpy_id++);

        _task_by_id.emplace (memcpy_id, task);
        m_queues[t.queue]->enqueue (task);

        return memcpy_id;
      }

      void transfer_manager_t::wait (type::memcpy_id_t const& memcpy_id)
      {
        auto const task_it (_task_by_id.find (memcpy_id));
        if (task_it == _task_by_id.end())
        {
          throw std::invalid_argument ("no such memcpy id");
        }

        FHG_UTIL_FINALLY ([&] { _task_by_id.erase (task_it); });

        task_it->second->get();
      }
    }
  }
}
