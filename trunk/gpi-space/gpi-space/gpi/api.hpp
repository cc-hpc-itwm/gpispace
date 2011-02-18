#ifndef GPI_SPACE_GPI_API_HPP
#define GPI_SPACE_GPI_API_HPP 1

#include <gpi-space/types.hpp>
#include <gpi-space/exception.hpp>

#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>

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

        virtual ~dma_error () throw () {}

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
      ~gpi_api_t();

      static gpi_api_t & create (int ac, char *av[]);
      static gpi_api_t & get ();
      static void destroy ();

      void set_memory_size (const gpi::size_t);

      // wrapped C function calls
      void start (const gpi::timeout_t timeout);
      void stop ();
      void kill ();
      void shutdown ();

      gpi::size_t number_of_counters () const;
      gpi::size_t number_of_queues () const;
      gpi::size_t queue_depth () const;
      gpi::version_t version () const;
      gpi::port_t port () const;
      gpi::size_t number_of_nodes () const;

      gpi::size_t open_dma_requests (const queue_desc_t) const;
      bool max_dma_requests_reached (const queue_desc_t) const;

      gpi::size_t open_passive_requests () const;
      bool max_passive_requests_reached () const;

      std::string hostname (const gpi::rank_t) const;
      gpi::rank_t rank () const;
      gpi::error_vector_t get_error_vector(const queue_desc_t) const;
      void *dma_ptr (void);

      template <typename T>
      T* dma_ptr (void) { return (T*)(dma_ptr()); }

      void set_network_type (const gpi::network_type_t);
      void set_port (const gpi::port_t);
      void set_mtu (const gpi::size_t);
      void set_number_of_processes (const gpi::size_t);

      bool ping (const gpi::rank_t) const;
      bool ping (std::string const & hostname) const;

      void check (const gpi::rank_t) const;
      void check () const;

      bool is_master (void) const;
      bool is_slave (void) const;

      void barrier (void) const;
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
      gpi_api_t ();

      void init (int ac, char *av[]);

      static gpi_api_t * instance;

      int startup_timedout_cb (int);

      int   m_ac;
      char **m_av;
      bool m_is_master;
      bool  m_startup_done;
      mutable rank_t m_rank;
      mutable size_t m_num_nodes;
      mutable size_t m_mem_size;
      void *m_dma;
    };
  }
}

#endif
