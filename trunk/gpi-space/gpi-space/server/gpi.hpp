#ifndef GPI_SPACE_SERVER_HPP
#define GPI_SPACE_SERVER_HPP 1

#include <gpi-space/types.hpp>

namespace gpi
{
  namespace server
  {
    class api_t
    {
    public:
      void init (int ac, char *av[]);

      void set_memory_size (const gpi::size_t);

      // wrapped C function calls
      void start (const gpi::timeout_t timeout);
      void stop ();
      void kill ();

      gpi::size_t number_of_counters () const;
      gpi::size_t number_of_queues () const;
      gpi::size_t queue_depth () const;
      gpi::version_t version () const;
      gpi::port_t port () const;
      gpi::size_t number_of_nodes () const;
      std::string hostname (const gpi::rank_t) const;
      gpi::rank_t rank () const;
      void * dma_ptr ();

      void set_network_type (const gpi::network_type_t);
      void set_port (const gpi::port_t);
      void set_mtu (const gpi::size_t);
      void set_number_of_processes (const gpi::size_t);

      bool ping (const gpi::rank_t) const;
      bool ping (std::string const & hostname) const;
      bool is_master () const;
      bool is_slave o() const;

      void barrier () const;
    };
  }
}

#endif
