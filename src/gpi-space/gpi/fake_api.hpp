#ifndef GPI_SPACE_FAKE_GPI_API_HPP
#define GPI_SPACE_FAKE_GPI_API_HPP 1

#include <vector>

#include <gpi-space/gpi/api.hpp>

#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace gpi
{
  namespace api
  {
    class fake_gpi_api_t : public gpi_api_t
    {
    public:
      fake_gpi_api_t (bool is_master);
      ~fake_gpi_api_t();

      // wrapped C function calls
      void clear_caches () {}
      void set_binary_path (const char *);
      void start (int ac, char *av[], const gpi::timeout_t timeout);
      void stop ();
      void kill ();
      void shutdown ();

      gpi::size_t number_of_counters () const;
      gpi::size_t number_of_queues () const;
      gpi::size_t queue_depth () const;
      gpi::version_t version () const;
      gpi::port_t port () const;
      gpi::size_t number_of_nodes () const;
      gpi::size_t memory_size () const;

      gpi::size_t open_dma_requests (const queue_desc_t) const;
      bool max_dma_requests_reached (const queue_desc_t) const;

      gpi::size_t open_passive_requests () const;
      bool max_passive_requests_reached () const;

      const char * hostname (const gpi::rank_t) const;
      gpi::rank_t rank () const;
      gpi::error_vector_t get_error_vector(const queue_desc_t) const;
      void *dma_ptr (void);

      template <typename T>
      T* dma_ptr (void) { return (T*)(dma_ptr()); }

      void set_network_type (const gpi::network_type_t);
      void set_port (const gpi::port_t);
      void set_mtu (const gpi::size_t);
      void set_number_of_processes (const gpi::size_t);
      void set_memory_size (const gpi::size_t);

      bool ping (const gpi::rank_t) const;
      bool ping (const char * host) const;

      int check (const char * host) const;
      int check (const gpi::rank_t) const;
      int check () const;

      bool is_master (void) const;
      bool is_slave (void) const;

      void barrier (void) const;
      bool try_lock (void) const;
      void lock (void) const;
      void unlock (void) const;

      void read_dma ( const offset_t local_offset
                    , const offset_t remote_offset
                    , const size_t amount
                    , const rank_t from_node
                    , const queue_desc_t queue
                    );
      void write_dma ( const offset_t local_offset
                     , const offset_t remote_offset
                     , const size_t amount
                     , const rank_t to_node
                     , const queue_desc_t queue
                     );

      void send_dma ( const offset_t local_offset
                    , const size_t amount
                    , const rank_t to_node
                    , const queue_desc_t queue
                    );
      void recv_dma ( const offset_t local_offset
                    , const size_t size
                    , const rank_t from_node
                    , const queue_desc_t queue
                    );

      size_t wait_dma (const queue_desc_t queue);

      void send_passive ( const offset_t local_offset
                        , const size_t amount
                        , const rank_t to_node
                        );
      void recv_passive ( const offset_t local_offset
                        , const size_t amount
                        , rank_t & from_node
                        );
      size_t wait_passive ( void );

    private:
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      mutable mutex_type m_mutex;
      bool m_is_master;
      const char *m_binary_path;
      bool  m_startup_done;
      rank_t m_rank;
      size_t m_mem_size;
      void *m_dma;

      // fake stuff
      size_t m_queue_count;
      std::vector<size_t> m_dma_request_count;
    };
  }
}

#endif