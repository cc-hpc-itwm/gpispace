#ifndef GPI_SPACE_GPI_API_HPP
#define GPI_SPACE_GPI_API_HPP 1

#include <gpi-space/types.hpp>
#include <gpi-space/exception.hpp>

#include <boost/shared_ptr.hpp>
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
      static const char * REAL_API;
      static const char * FAKE_API;

      static gpi_api_t & create (std::string const & impl);
      static gpi_api_t & get ();
      static void destroy ();

      virtual ~gpi_api_t() { }

      // wrapped C function calls
      virtual void set_binary_path (const char *) = 0;
      virtual void clear_caches () = 0;
      virtual int build_hostlist () = 0;
      virtual void start (int ac, char *av[], const gpi::timeout_t timeout) = 0;
      virtual void stop () = 0;
      virtual void kill () = 0;
      virtual void shutdown () = 0;

      virtual gpi::size_t number_of_counters () const = 0;
      virtual gpi::size_t number_of_queues () const = 0;
      virtual gpi::size_t queue_depth () const = 0;
      virtual gpi::version_t version () const = 0;
      virtual gpi::port_t port () const = 0;
      virtual gpi::size_t number_of_nodes () const = 0;
      virtual gpi::size_t memory_size () const = 0;

      virtual gpi::size_t open_dma_requests (const queue_desc_t) const = 0;
      virtual bool max_dma_requests_reached (const queue_desc_t) const = 0;

      virtual gpi::size_t open_passive_requests () const = 0;
      virtual bool max_passive_requests_reached () const = 0;

      virtual const char * hostname (const gpi::rank_t) const = 0;
      virtual gpi::rank_t rank () const = 0;
      virtual gpi::error_vector_t get_error_vector(const queue_desc_t) const = 0;
      virtual void *dma_ptr (void) = 0;

      template <typename T>
      T* dma_ptr (void) { return (T*)(dma_ptr()); }

      virtual void set_network_type (const gpi::network_type_t) = 0;
      virtual void set_port (const gpi::port_t) = 0;
      virtual void set_mtu (const gpi::size_t) = 0;
      virtual void set_number_of_processes (const gpi::size_t) = 0;
      virtual void set_memory_size (const gpi::size_t) = 0;

      virtual bool ping (const gpi::rank_t) const = 0;
      virtual bool ping (const char * host) const = 0;

      virtual int check (gpi::rank_t) const = 0;
      virtual int check (const char * hostname) const = 0;
      virtual int check () const = 0;

      virtual void set_is_master(const bool b) = 0;
      virtual bool is_master (void) const = 0;
      virtual bool is_slave (void) const = 0;

      virtual void barrier (void) const = 0;
      virtual bool try_lock (void) const = 0;
      virtual void lock (void) const = 0;
      virtual void unlock (void) const = 0;

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

      virtual void send_dma ( const offset_t local_offset
                    , const size_t amount
                    , const rank_t to_node
                    , const queue_desc_t queue
                    ) = 0;
      virtual void recv_dma ( const offset_t local_offset
                    , const size_t size
                    , const rank_t from_node
                    , const queue_desc_t queue
                    ) = 0;

      virtual size_t wait_dma (const queue_desc_t queue) = 0;

      virtual void send_passive ( const offset_t local_offset
                        , const size_t amount
                        , const rank_t to_node
                        ) = 0;
      virtual void recv_passive ( const offset_t local_offset
                        , const size_t amount
                        , rank_t & from_node
                        ) = 0;
      virtual size_t wait_passive ( void ) = 0;

    private:
      static boost::shared_ptr<gpi_api_t> instance;
    };
  }
}

#endif
