#include <gpi-space/gpi/gaspi.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhg/assert.hpp>

#include <gpi-space/exception.hpp>
#include <gpi-space/gpi/system.hpp>

#include <util-generic/hostname.hpp>

#include <GASPI.h>

#include <limits>

namespace gpi
{
  namespace api
  {
    namespace
    {
      template <typename Fun, typename... T>
      void fail_on_non_zero ( const std::string& function_name
                            , Fun&& f
                            , T&&... arguments
                            )
      {
        gaspi_return_t const rc (f (arguments...));
        if (rc != 0)
        {
          throw gpi::exception::gpi_error
            ( gpi::error::internal_error()
            , function_name + " failed: " + gaspi_error_str (rc)
            + " (" + std::to_string (rc) + ")"
            );
        }
      }

#define FAIL_ON_NON_ZERO(F, Args...)             \
      fail_on_non_zero(#F, F, Args)
    }

    gaspi_t::gaspi_t ( fhg::log::Logger& logger
                     , const unsigned long long memory_size
                     , const unsigned short port
                     , const std::chrono::seconds& timeout
                     , unsigned short communication_port
                     )
      : _logger (logger)
      , m_mem_size (memory_size)
      , m_dma (nullptr)
      , m_replacement_gpi_segment (0)
    {
      gaspi_config_t config;
      FAIL_ON_NON_ZERO (gaspi_config_get, &config);
      config.sn_port = port;
      FAIL_ON_NON_ZERO (gaspi_config_set, config);

      if (sys::get_total_memory_size() < m_mem_size)
      {
        throw gpi::exception::gpi_error
          ( gpi::error::startup_failed()
          , "not enough memory: requested memory size ("
          + std::to_string (m_mem_size) + ") exceeds total memory size ("
          + std::to_string (sys::get_total_memory_size()) + ")"
          );
      }
      else if (sys::get_avail_memory_size() < m_mem_size)
      {
        LLOG( WARN
            , _logger
           , "requested memory size (" << m_mem_size << ")"
           <<" exceeds available memory size (" << sys::get_avail_memory_size() << ")"
           );
      }

      FAIL_ON_NON_ZERO ( gaspi_proc_init
                       , (std::chrono::duration_cast<std::chrono::milliseconds> (timeout)).count()
                       );
      FAIL_ON_NON_ZERO ( gaspi_segment_create
                       , m_replacement_gpi_segment
                       , m_mem_size
                       , GASPI_GROUP_ALL
                       , GASPI_BLOCK
                       , GASPI_MEM_UNINITIALIZED
                       );
      FAIL_ON_NON_ZERO ( gaspi_segment_ptr
                       , m_replacement_gpi_segment
                       , &m_dma
                       );

      struct hostname_and_port_t
      {
        char hostname[HOST_NAME_MAX + 1]; // + \0
        unsigned short port;
      };

      const gaspi_segment_id_t exchange_hostname_and_port_segment {1};

      FAIL_ON_NON_ZERO ( gaspi_segment_create
                       , exchange_hostname_and_port_segment
                       , 2 * sizeof (hostname_and_port_t)
                       , GASPI_GROUP_ALL
                       , GASPI_BLOCK
                       , GASPI_MEM_UNINITIALIZED
                       );
      void* exchange_hostname_and_port_data_raw;
      FAIL_ON_NON_ZERO ( gaspi_segment_ptr
                       , exchange_hostname_and_port_segment
                       , &exchange_hostname_and_port_data_raw
                       );

      hostname_and_port_t* exchange_hostname_and_port_data_send
        (static_cast<hostname_and_port_t*> (exchange_hostname_and_port_data_raw));
      hostname_and_port_t* exchange_hostname_and_port_data_receive
        (exchange_hostname_and_port_data_send + 1);

      strncpy ( exchange_hostname_and_port_data_send->hostname
              , fhg::util::hostname().c_str()
              , sizeof (exchange_hostname_and_port_data_send->hostname)
              );
      exchange_hostname_and_port_data_send->port = communication_port;

      FAIL_ON_NON_ZERO (gaspi_barrier, GASPI_GROUP_ALL, GASPI_BLOCK);

      m_rank_to_hostname.resize (number_of_nodes());
      _communication_port_by_rank.resize (number_of_nodes());

      for ( gaspi_rank_t r (0)
          ; r < number_of_nodes()
          ; ++r
          )
      {
        memset ( exchange_hostname_and_port_data_receive
               , 0
               , sizeof (hostname_and_port_t)
               );

        FAIL_ON_NON_ZERO ( gaspi_read
                         , exchange_hostname_and_port_segment
                         , sizeof (hostname_and_port_t)
                         , r
                         , exchange_hostname_and_port_segment
                         , 0
                         , sizeof (hostname_and_port_t)
                         , 0
                         , GASPI_BLOCK
                         );
        FAIL_ON_NON_ZERO (gaspi_wait, 0, GASPI_BLOCK);

        m_rank_to_hostname[r] =
          exchange_hostname_and_port_data_receive->hostname;
        _communication_port_by_rank[r] =
          exchange_hostname_and_port_data_receive->port;
      }

      FAIL_ON_NON_ZERO (gaspi_barrier, GASPI_GROUP_ALL, GASPI_BLOCK);
      FAIL_ON_NON_ZERO (gaspi_segment_delete, exchange_hostname_and_port_segment);
    }

    gaspi_t::~gaspi_t()
    {
      FAIL_ON_NON_ZERO (gaspi_proc_term, GASPI_BLOCK);
    }

    gpi::size_t gaspi_t::number_of_queues() const
    {
      gaspi_number_t queue_num;
      FAIL_ON_NON_ZERO (gaspi_queue_num, &queue_num);
      return queue_num;
    }

    gpi::size_t gaspi_t::queue_depth() const
    {
      gaspi_number_t queue_size_max;
      FAIL_ON_NON_ZERO (gaspi_queue_size_max,  &queue_size_max);
      return queue_size_max;
    }

    gpi::version_t gaspi_t::version() const
    {
      float vsn;
      FAIL_ON_NON_ZERO (gaspi_version, &vsn);
      return vsn;
    }

    gpi::port_t gaspi_t::port() const
    {
      gaspi_config_t config;
      FAIL_ON_NON_ZERO (gaspi_config_get, &config);
      return config.sn_port;
    }

    gpi::size_t gaspi_t::number_of_nodes() const
    {
      gaspi_rank_t num_ranks;
      FAIL_ON_NON_ZERO (gaspi_proc_num, &num_ranks);
      return num_ranks;
    }

    gpi::size_t gaspi_t::memory_size() const
    {
      return m_mem_size;
    }

    gpi::size_t gaspi_t::max_transfer_size() const
    {
      gaspi_size_t transfer_size_max;
      FAIL_ON_NON_ZERO (gaspi_transfer_size_max, &transfer_size_max);
      return transfer_size_max;
    }

    gpi::size_t gaspi_t::open_dma_requests (const queue_desc_t q) const
    {
      gaspi_number_t queue_size;
      FAIL_ON_NON_ZERO (gaspi_queue_size, q, &queue_size);
      return queue_size;
    }

    bool gaspi_t::max_dma_requests_reached (const queue_desc_t q) const
    {
      return (open_dma_requests (q) >= queue_depth());
    }

    gpi::rank_t gaspi_t::rank() const
    {
      gaspi_rank_t rank;
      FAIL_ON_NON_ZERO (gaspi_proc_rank, &rank);
      return rank;
    }

    std::string const& gaspi_t::hostname_of_rank (const gpi::rank_t r) const
    {
      return m_rank_to_hostname[r];
    }

    unsigned short gaspi_t::communication_port_of_rank (gpi::rank_t rank) const
    {
      return _communication_port_by_rank[rank];
    }

    gpi::error_vector_t gaspi_t::get_error_vector (const gpi::queue_desc_t) const
    {
      std::vector<unsigned char> gaspi_state_vector (number_of_nodes());
      FAIL_ON_NON_ZERO (gaspi_state_vec_get, gaspi_state_vector.data());

      gpi::error_vector_t v (gaspi_state_vector.size());
      for (std::size_t i (0); i < number_of_nodes(); ++i)
      {
        v.set (i, (gaspi_state_vector [i] != 0));
      }
      return v;
    }

    void * gaspi_t::dma_ptr (void)
    {
      return m_dma;
    }

    void gaspi_t::read_dma ( const offset_t local_offset
                           , const offset_t remote_offset
                           , const size_t amount
                           , const rank_t from_node
                           , const queue_desc_t queue
                           )
    {
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

        try
        {
          FAIL_ON_NON_ZERO ( gaspi_read
                           , m_replacement_gpi_segment
                           , l_off
                           , from_node
                           , m_replacement_gpi_segment
                           , r_off
                           , to_transfer
                           , queue
                           , GASPI_BLOCK
                           );
        }
        catch (gpi::exception::gpi_error const &e)
        {
          throw exception::dma_error
            ( gpi::error::read_dma_failed (e.user_message)
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

        try
        {
          FAIL_ON_NON_ZERO ( gaspi_write
                           , m_replacement_gpi_segment
                           , l_off
                           , to_node
                           , m_replacement_gpi_segment
                           , r_off
                           , to_transfer
                           , queue
                           , GASPI_BLOCK
                           );
        }
        catch (const gpi::exception::gpi_error& e)
        {
          throw exception::dma_error
            ( gpi::error::write_dma_failed (e.user_message)
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

    void gaspi_t::wait_dma (const queue_desc_t queue)
    {
      FAIL_ON_NON_ZERO (gaspi_wait, queue, GASPI_BLOCK);
    }

#undef FAIL_ON_NON_ZERO
  }
}
