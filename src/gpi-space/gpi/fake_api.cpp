#include <gpi-space/gpi/fake_api.hpp>

#include <gpi-space/log_to_GLOBAL_logger.hpp>

#include <gpi-space/exception.hpp>

#include <gpi-space/gpi/system.hpp>

#include <fhg/util/hostname.hpp>

namespace gpi
{
  namespace api
  {
    fake_gpi_api_t::fake_gpi_api_t ( const unsigned long long memory_size
                                   , const std::chrono::seconds&
                                   , unsigned short communication_port
                                   )
      : m_rank (0)
      , m_mem_size (memory_size)
      , m_dma (nullptr)
      , m_hostname (fhg::util::hostname())
      , _communication_port (communication_port)
    {
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

      m_dma = malloc(m_mem_size * sizeof (char));
      if (! m_dma)
      {
        throw gpi::exception::gpi_error
          ( gpi::error::startup_failed()
          , "could not allocate memory"
          );
      }
      memset (m_dma, 0, m_mem_size);
    }

    fake_gpi_api_t::~fake_gpi_api_t ()
    {
      free (m_dma); m_dma = nullptr;
    }

    // wrapped C function calls
    gpi::size_t fake_gpi_api_t::number_of_queues () const
    {
      return NUMBER_OF_SIMULATED_QUEUES;
    }

    gpi::size_t fake_gpi_api_t::queue_depth () const
    {
      return 1024;
    }

    gpi::version_t fake_gpi_api_t::version () const
    {
      return 0.01;
    }

    gpi::port_t fake_gpi_api_t::port () const
    {
      return 42423;
    }

    gpi::size_t fake_gpi_api_t::number_of_nodes () const
    {
      return 1;
    }

    gpi::size_t fake_gpi_api_t::memory_size () const
    {
      return m_mem_size;
    }

    gpi::size_t fake_gpi_api_t::open_dma_requests (const queue_desc_t q) const
    {
      return m_dma_request_count[q];
    }

    bool fake_gpi_api_t::max_dma_requests_reached (const queue_desc_t q) const
    {
      return (open_dma_requests(q) >= queue_depth());
    }

    gpi::rank_t fake_gpi_api_t::rank () const
    {
      return m_rank;
    }

    std::string const& fake_gpi_api_t::hostname_of_rank (const gpi::rank_t) const
    {
      return m_hostname;
    }

    unsigned short fake_gpi_api_t::communication_port_of_rank (gpi::rank_t) const
    {
      return _communication_port;
    }

    gpi::error_vector_t fake_gpi_api_t::get_error_vector (const gpi::queue_desc_t) const
    {
      gpi::error_vector_t v (number_of_nodes());
      for (std::size_t i (0); i < number_of_nodes(); ++i)
      {
        v.set (i, 0);
      }
      return v;
    }

    void * fake_gpi_api_t::dma_ptr (void)
    {
      return m_dma;
    }

    void fake_gpi_api_t::read_dma ( const offset_t local_offset
                                  , const offset_t remote_offset
                                  , const size_t amount
                                  , const rank_t from_node
                                  , const queue_desc_t queue
                                  )
    {
      if (from_node != 0)
      {
        throw exception::dma_error
          ( gpi::error::read_dma_failed()
          , local_offset
          , remote_offset
          , from_node
          , rank()
          , amount
          , queue
          );
      }
      else
      {
        if (m_dma_request_count[queue] >= queue_depth())
        {
          throw exception::dma_error
            ( gpi::error::read_dma_failed()
            , local_offset
            , remote_offset
            , from_node
            , rank()
            , amount
            , queue
            );
        }
        ++m_dma_request_count[queue];

        memcpy ( (char*)m_dma + local_offset
               , (char*)m_dma + remote_offset
               , amount
               );
      }
    }
    void fake_gpi_api_t::write_dma ( const offset_t local_offset
                                   , const offset_t remote_offset
                                   , const size_t amount
                                   , const rank_t to_node
                                   , const queue_desc_t queue
                                   )
    {
      if (to_node != 0)
      {
        throw exception::dma_error
          ( gpi::error::write_dma_failed()
          , local_offset
          , remote_offset
          , rank()
          , to_node
          , amount
          , queue
          );
      }
      else
      {
        if (m_dma_request_count[queue] >= queue_depth())
        {
          throw exception::dma_error
            ( gpi::error::write_dma_failed()
            , local_offset
            , remote_offset
            , rank()
            , to_node
            , amount
            , queue
          );
        }
        ++m_dma_request_count[queue];

        memcpy ( (char*)m_dma + remote_offset
               , (char*)m_dma + local_offset
               , amount
               );
      }
    }

    void fake_gpi_api_t::wait_dma (const queue_desc_t queue)
    {
      m_dma_request_count[queue] = 0;
    }
  }
}
