#include <gpi-space/gpi/gaspi.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhg/assert.hpp>

#include <gpi-space/exception.hpp>
#include <gpi-space/gpi/system.hpp>

#include <GASPI.h>

#include <csignal> // sigwait
#include <cstring> // strerror
#include <limits>

namespace gpi
{
  namespace api
  {
    gaspi_t::gaspi_t (bool is_master, const unsigned long long memory_size, const unsigned short port)
      : m_is_master (is_master)
      , m_startup_done (false)
      , m_mem_size (memory_size)
      , m_dma (nullptr)
      , m_replacement_gpi_segment (0)
    {
      gaspi_config_t config;
      gaspi_config_get (&config);
      config.sn_port = port;
      gaspi_config_set (config);
    }

    gaspi_t::~gaspi_t()
    {
      stop();
    }

    // wrapped C function calls
    void gaspi_t::start (int, char *[], const gpi::timeout_t timeout)
    {
      fhg_assert (! m_startup_done);

      if (sys::get_total_memory_size() < m_mem_size)
      {
        LOG( ERROR
           , "requested memory size (" << m_mem_size << ")"
           <<" exceeds total memory size (" << sys::get_total_memory_size() << ")"
           );
        throw gpi::exception::gpi_error
          ( gpi::error::startup_failed()
          , "not enough memory"
          );
      }
      else if (sys::get_avail_memory_size() < m_mem_size)
      {
        LOG( WARN
           , "requested memory size (" << m_mem_size << ")"
           <<" exceeds available memory size (" << sys::get_avail_memory_size() << ")"
           );
      }

      gaspi_return_t rc (gaspi_proc_init (timeout * 1000));

      if (rc == GASPI_ERROR)
      {
        throw gpi::exception::gpi_error
          ( gpi::error::startup_failed()
          , "gaspi_proc_init() failed"
          );
      }
      else if (rc == GASPI_TIMEOUT)
      {
        throw gpi::exception::gpi_error
          ( gpi::error::startup_failed()
          , "gaspi_proc_init() timed out"
          );
      }

      if (GASPI_ERROR == gaspi_segment_create ( m_replacement_gpi_segment
                                              , m_mem_size
                                              , GASPI_GROUP_ALL
                                              , GASPI_BLOCK
                                              , GASPI_MEM_UNINITIALIZED
                                              )
         )
      {
        throw gpi::exception::gpi_error
          ( gpi::error::startup_failed()
          , "gaspi_segment_create() failed"
          );
      }

      if (GASPI_ERROR == gaspi_segment_ptr (m_replacement_gpi_segment, &m_dma))
      {
        throw gpi::exception::gpi_error
          ( gpi::error::startup_failed()
          , "gaspi_segment_ptr() failed"
          );
      }

      m_startup_done = true;
    }

    void gaspi_t::stop()
    {
      lock_type lock (m_mutex);
      if (m_startup_done)
      {
        if (GASPI_ERROR == gaspi_proc_term (GASPI_BLOCK))
        {
          throw gpi::exception::gpi_error
            ( gpi::error::internal_error()
            , "gaspi_t::gaspi_proc_term() failed"
            );
        }
        m_startup_done = false;
      }
    }

    void gaspi_t::shutdown()
    {
      stop();
    }

    gpi::size_t gaspi_t::number_of_queues() const
    {
      gaspi_number_t queue_num;
      if (GASPI_ERROR == gaspi_queue_num (&queue_num))
      {
        throw gpi::exception::gpi_error
          ( gpi::error::internal_error()
          , "gaspi_t::number_of_queues() failed"
          );
      }
      return queue_num;
    }

    gpi::size_t gaspi_t::queue_depth() const
    {
      gaspi_number_t queue_size_max;
      if (GASPI_ERROR == gaspi_queue_size_max (&queue_size_max))
      {
        throw gpi::exception::gpi_error
          ( gpi::error::internal_error()
          , "gaspi_t::queue_depth() failed"
          );
      }
      return queue_size_max;
    }

    gpi::version_t gaspi_t::version() const
    {
      float vsn;
      if (GASPI_ERROR == gaspi_version (&vsn))
      {
        throw gpi::exception::gpi_error
          ( gpi::error::internal_error()
          , "gaspi_t::version()"
          );
      }
      return vsn;
    }

    gpi::port_t gaspi_t::port() const
    {
      gaspi_config_t config;
      gaspi_config_get (&config);
      return config.sn_port;
    }

    gpi::size_t gaspi_t::number_of_nodes() const
    {
      gaspi_rank_t num_ranks;
      if (GASPI_ERROR == gaspi_proc_num (&num_ranks))
      {
        throw gpi::exception::gpi_error
          ( gpi::error::internal_error()
          , "gaspi_t::number_of_nodes() failed"
          );
      }
      return num_ranks;
    }

    gpi::size_t gaspi_t::memory_size() const
    {
      return m_mem_size;
    }

    gpi::size_t gaspi_t::max_transfer_size() const
    {
      gaspi_size_t transfer_size_max;
      if (GASPI_ERROR == gaspi_transfer_size_max (&transfer_size_max))
      {
        throw gpi::exception::gpi_error
          ( gpi::error::internal_error()
          , "gaspi_t::max_transfer_size() failed"
          );
      }
      return transfer_size_max;
    }

    gpi::size_t gaspi_t::open_dma_requests (const queue_desc_t q) const
    {
      fhg_assert (m_startup_done);

      gaspi_number_t queue_size;
      if (GASPI_ERROR == gaspi_queue_size (q, &queue_size))
      {
        throw gpi::exception::gpi_error
          ( gpi::error::internal_error()
          , "gaspi_t::open_dma_requests() failed"
          );
      }
      return queue_size;
    }

    bool gaspi_t::max_dma_requests_reached (const queue_desc_t q) const
    {
      return (open_dma_requests (q) >= queue_depth());
    }

    gpi::rank_t gaspi_t::rank() const
    {
      fhg_assert (m_startup_done);
      gaspi_rank_t rank;
      if (GASPI_ERROR == gaspi_proc_rank (&rank))
      {
        throw gpi::exception::gpi_error
          ( gpi::error::internal_error()
          , "gaspi_t::rank() failed"
          );
      }
      return rank;
    }

    gpi::error_vector_t gaspi_t::get_error_vector (const gpi::queue_desc_t) const
    {
      fhg_assert (m_startup_done);
      std::vector<unsigned char> gaspi_state_vector (number_of_nodes());
      if (GASPI_ERROR == gaspi_state_vec_get (gaspi_state_vector.data()))
      {
        throw gpi::exception::gpi_error
          ( gpi::error::internal_error()
          , "gaspi_t::get_error_vector() failed"
          );
      }

      gpi::error_vector_t v (gaspi_state_vector.size());
      for (std::size_t i (0); i < number_of_nodes(); ++i)
      {
        v.set (i, (gaspi_state_vector [i] != 0));
      }
      return v;
    }

    void * gaspi_t::dma_ptr (void)
    {
      fhg_assert (m_startup_done);
      return m_dma;
    }

    bool gaspi_t::is_master (void) const
    {
      return m_is_master;
    }
    bool gaspi_t::is_slave (void) const
    {
      return !is_master();
    }

    void gaspi_t::read_dma ( const offset_t local_offset
                           , const offset_t remote_offset
                           , const size_t amount
                           , const rank_t from_node
                           , const queue_desc_t queue
                           )
    {
      fhg_assert (m_startup_done);

      size_t remaining (amount);
      const size_t chunk_size (max_transfer_size());

      size_t l_off (local_offset);
      size_t r_off (remote_offset);

      while (remaining)
      {
        const size_t to_transfer (std::min (chunk_size, remaining));

        if (max_dma_requests_reached (queue))
        {
          wait_dma (queue);
        }

        if (GASPI_ERROR == gaspi_read ( m_replacement_gpi_segment
                                      , l_off
                                      , from_node
                                      , m_replacement_gpi_segment
                                      , r_off
                                      , to_transfer
                                      , queue
                                      , GASPI_BLOCK
                                      )
           )
        {
          throw exception::dma_error
            ( gpi::error::read_dma_failed()
            , l_off
            , r_off
            , from_node
            , rank()
            , to_transfer
            , queue
            );
        }

        remaining -= to_transfer;
        l_off     += to_transfer;
        r_off     += to_transfer;
      }
    }

    void gaspi_t::write_dma ( const offset_t local_offset
                            , const offset_t remote_offset
                            , const size_t amount
                            , const rank_t to_node
                            , const queue_desc_t queue
                            )
    {
      fhg_assert (m_startup_done);

      size_t remaining (amount);
      const size_t chunk_size (max_transfer_size());

      size_t l_off (local_offset);
      size_t r_off (remote_offset);

      while (remaining)
      {
        const size_t to_transfer (std::min (chunk_size, remaining));

        if (max_dma_requests_reached (queue))
        {
          wait_dma (queue);
        }

        if (GASPI_ERROR == gaspi_write ( m_replacement_gpi_segment
                                       , l_off
                                       , to_node
                                       , m_replacement_gpi_segment
                                       , r_off
                                       , to_transfer
                                       , queue
                                       , GASPI_BLOCK
                                       )
           )
        {
          throw exception::dma_error
            ( gpi::error::write_dma_failed()
            , l_off
            , r_off
            , to_node
            , rank()
            , to_transfer
            , queue
            );
        }

        remaining -= to_transfer;
        l_off     += to_transfer;
        r_off     += to_transfer;
      }
    }

    size_t gaspi_t::wait_dma (const queue_desc_t queue)
    {
      fhg_assert (m_startup_done);
      if (GASPI_ERROR == gaspi_wait (queue, GASPI_BLOCK))
      {
        throw exception::dma_error
          ( gpi::error::wait_dma_failed()
          , 0
          , 0
          , rank()
          , 0
          , 0
          , queue
          );
      }
      return 0; // not sure what to return here, 0 or something > 0
    }
  }
}
