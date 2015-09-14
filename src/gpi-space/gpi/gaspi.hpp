#pragma once

#include <fhglog/Logger.hpp>

#include <gpi-space/gpi/api.hpp>

#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <vector>

namespace gpi
{
  namespace api
  {
    class gaspi_t : public gpi_api_t
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
      virtual gpi::size_t number_of_queues () const override;
      virtual gpi::size_t queue_depth () const override;
      virtual gpi::version_t version () const override;
      virtual gpi::size_t number_of_nodes () const override;
      virtual gpi::size_t memory_size () const override;
      virtual gpi::size_t max_transfer_size () const;

      virtual gpi::size_t open_dma_requests (const queue_desc_t) const override;
      virtual bool max_dma_requests_reached (const queue_desc_t) const override;

      virtual gpi::rank_t rank () const override;
      virtual std::string const& hostname_of_rank (const gpi::rank_t) const override;
      virtual unsigned short communication_port_of_rank (gpi::rank_t) const override;
      virtual gpi::error_vector_t get_error_vector(const queue_desc_t) const override;
      virtual void *dma_ptr (void) override;

      template <typename T>
      T* dma_ptr (void) { return (T*)(dma_ptr()); }

      virtual void read_dma ( const offset_t local_offset
                    , const offset_t remote_offset
                    , const size_t amount
                    , const rank_t from_node
                    , const queue_desc_t queue
                    ) override;
      virtual void write_dma ( const offset_t local_offset
                     , const offset_t remote_offset
                     , const size_t amount
                     , const rank_t to_node
                     , const queue_desc_t queue
                     ) override;
      virtual void wait_dma (const queue_desc_t queue) override;
    private:
      fhg::log::Logger& _logger;
      size_t m_mem_size;
      void *m_dma;
      size_t m_replacement_gpi_segment;
      std::vector<std::string> m_rank_to_hostname;
      std::vector<unsigned short> _communication_port_by_rank;
    };
  }
}
