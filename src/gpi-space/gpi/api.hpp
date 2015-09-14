#pragma once

#include <gpi-space/types.hpp>
#include <gpi-space/exception.hpp>

#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>

#include <chrono>

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

    class gpi_api_t : boost::noncopyable
    {
    public:
      virtual ~gpi_api_t() = default;

      // wrapped C function calls
      virtual gpi::size_t number_of_queues () const = 0;
      virtual gpi::size_t queue_depth () const = 0;
      virtual gpi::version_t version () const = 0;
      virtual gpi::size_t number_of_nodes () const = 0;
      virtual gpi::size_t memory_size () const = 0;

      virtual gpi::size_t open_dma_requests (const queue_desc_t) const = 0;
      virtual bool max_dma_requests_reached (const queue_desc_t) const = 0;

      virtual gpi::rank_t rank () const = 0;
      virtual std::string const& hostname_of_rank (const gpi::rank_t) const = 0;
      virtual unsigned short communication_port_of_rank (gpi::rank_t) const = 0;
      virtual gpi::error_vector_t get_error_vector(const queue_desc_t) const = 0;
      virtual void *dma_ptr (void) = 0;

      template <typename T>
      T* dma_ptr (void) { return (T*)(dma_ptr()); }

      virtual void read_dma ( const offset_t local_offset
                    , const offset_t remote_offset
                    , const size_t amount
                    , const rank_t from_node
                    , const queue_desc_t queue
                    ) = 0;
      virtual void write_dma ( const offset_t local_offset
                     , const offset_t remote_offset
                     , const size_t amount
                     , const rank_t to_node
                     , const queue_desc_t queue
                     ) = 0;

      virtual void wait_dma (const queue_desc_t queue) = 0;
    };
  }
}
