#ifndef GPI_SPACE_GASPI_API_HPP
#define GPI_SPACE_GASPI_API_HPP 1

#include <gpi-space/gpi/api.hpp>

#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace gpi
{
  namespace api
  {
    class gaspi_t : public gpi_api_t
    {
    public:
      gaspi_t (bool is_master);
      ~gaspi_t();

      // wrapped C function calls
      virtual void start (int ac, char *av[], const gpi::timeout_t timeout) override;
      virtual void stop () override;
      virtual void kill () override;
      virtual void shutdown () override;

      virtual gpi::size_t number_of_counters () const override;
      virtual gpi::size_t number_of_queues () const override;
      virtual gpi::size_t queue_depth () const override;
      virtual gpi::version_t version () const override;
      virtual gpi::port_t port () const override;
      virtual gpi::size_t number_of_nodes () const override;
      virtual gpi::size_t memory_size () const override;
      virtual gpi::size_t max_transfer_size () const;

      virtual gpi::size_t open_dma_requests (const queue_desc_t) const override;
      virtual bool max_dma_requests_reached (const queue_desc_t) const override;

      virtual gpi::size_t open_passive_requests () const override;
      virtual bool max_passive_requests_reached () const override;

      virtual const char * hostname (const gpi::rank_t) const override;
      virtual gpi::rank_t rank () const override;
      virtual gpi::error_vector_t get_error_vector(const queue_desc_t) const override;
      virtual void *dma_ptr (void) override;

      template <typename T>
      T* dma_ptr (void) { return (T*)(dma_ptr()); }

      virtual void set_memory_size (const gpi::size_t) override;

      virtual bool ping (const gpi::rank_t) const override;
      virtual bool ping (const char * hostname) const override;

      virtual bool is_master (void) const override;
      virtual bool is_slave (void) const override;

      virtual void barrier (void) const override;
      virtual bool try_lock (void) const override;
      virtual void lock (void) const override;
      virtual void unlock (void) const override;

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

      virtual void send_dma ( const offset_t local_offset
                    , const size_t amount
                    , const rank_t to_node
                    , const queue_desc_t queue
                    ) override;
      virtual void recv_dma ( const offset_t local_offset
                    , const size_t size
                    , const rank_t from_node
                    , const queue_desc_t queue
                    ) override;

      virtual size_t wait_dma (const queue_desc_t queue) override;

      virtual void send_passive ( const offset_t local_offset
                        , const size_t amount
                        , const rank_t to_node
                        ) override;
      virtual void recv_passive ( const offset_t local_offset
                        , const size_t amount
                        , rank_t & from_node
                        ) override;
      virtual size_t wait_passive ( void ) override;

    private:
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      mutable mutex_type m_mutex;
      bool m_is_master;
      bool  m_startup_done;
      size_t m_mem_size;
      void *m_dma;
      size_t m_replacement_gpi_segment;
    };
  }
}

#endif
