#pragma once

#include <fhglog/Logger.hpp>

#include <gpi-space/exception.hpp>
#include <gpi-space/types.hpp>

#include <boost/noncopyable.hpp>

#include <chrono>
#include <string>
#include <vector>

namespace gpi
{
  namespace api
  {
    namespace exception
    {
      struct dma_error : public gpi::exception::gpi_error
      {
        dma_error ( gpi::error::code_t const & ec
                  , const offset_t loc_offset
                  , const offset_t rem_offset
                  , const rank_t from
                  , const rank_t to
                  , const size_t bytes
                  , const queue_desc_t via_queue
                  )
          : gpi::exception::gpi_error (ec)
          , local_offset (loc_offset)
          , remote_offset (rem_offset)
          , from_node (from)
          , to_node (to)
          , amount (bytes)
          , queue (via_queue)
        {}

        const offset_t local_offset;
        const offset_t remote_offset;
        const rank_t from_node;
        const rank_t to_node;
        const size_t amount;
        const queue_desc_t queue;
      };
    }

    class gaspi_t : boost::noncopyable
    {
    public:
      gaspi_t ( fhg::log::Logger&
              , const unsigned long long memory_size
              , const unsigned short port
              , const std::chrono::seconds& timeout
              , unsigned short communication_port
              );
      ~gaspi_t();

      // wrapped C function calls
      gpi::size_t number_of_queues () const;
      gpi::size_t queue_depth () const;
      gpi::size_t number_of_nodes () const;
      gpi::size_t memory_size () const;
      gpi::size_t max_transfer_size () const;

      gpi::rank_t rank () const;
      std::string const& hostname_of_rank (const gpi::rank_t) const;
      unsigned short communication_port_of_rank (gpi::rank_t) const;
      gpi::error_vector_t get_error_vector(const queue_desc_t) const;
      void *dma_ptr (void);

      template <typename T>
      T* dma_ptr (void) { return (T*)(dma_ptr()); }

      struct read_dma_info
      {
        queue_desc_t queue;
      };
      read_dma_info read_dma ( const offset_t local_offset
                             , const offset_t remote_offset
                             , const size_t amount
                             , const rank_t from_node
                             , const queue_desc_t queue
                             );
      void wait_readable (std::list<read_dma_info> const&);

      void write_dma ( const offset_t local_offset
                     , const offset_t remote_offset
                     , const size_t amount
                     , const rank_t to_node
                     , const queue_desc_t queue
                     );
      void wait_dma (const queue_desc_t queue);
    private:
      gpi::size_t open_dma_requests (const queue_desc_t) const;
      bool max_dma_requests_reached (const queue_desc_t q) const
      {
        return (open_dma_requests (q) >= queue_depth());
      }
      fhg::log::Logger& _logger;
      size_t m_mem_size;
      void *m_dma;
      size_t m_replacement_gpi_segment;
      std::vector<std::string> m_rank_to_hostname;
      std::vector<unsigned short> _communication_port_by_rank;
    };
  }
}
