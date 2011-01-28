#define LOG_COMPONENT gpi
#include <fhglog/minimal.hpp>

#include <limits>

#include "gpi.hpp"

#include <GPI/GPI.h>

namespace gpi
{
  namespace server
  {
    namespace signal_handler
    {
      template <typename API, void (API::*sig_handler)(int)>
      struct timeout_handler_t
      {
        typedef API api_t;

        timeout_handler_t (api_t & api)
          : m_api (api)
        {}

        void operator () (int s)
        {
          m_api.*sig_handler (s);
        }
      private:
        api_t & m_api;
      };
    }

    namespace real
    {
      gpi_api_t::gpi_api_t ()
        : m_ac (0)
        , m_av (0)
        , m_startup_done (false)
        , m_rank (std::numeric_limits<rank_t>::max())
        , m_num_nodes (0)
        , m_mem_size (0)
      { }

      gpi_api_t::~gpi_api_t ()
      {
        try
        {
          stop ();
        }
        catch (std::exception const & ex)
        {
          LOG(WARN, "could not stop real::gpi_api_t on rank " << rank());
        }
      }

      void gpi_api_t::init (int ac, char *av[])
      {
        m_ac = ac;
        m_av = av;
      }

      void gpi_api_t::set_memory_size (const gpi::size_t sz)
      {
        m_mem_size = sz;
      }

      // wrapped C function calls
      void gpi_api_t::start (const gpi::timeout_t timeout)
      {
        // 1. register alarm signal handler
        // 2. start alarm
        // 3. start gpi
        // 4a. if alarm before start_gpi returns
        //        - restore old alarm handler
        //        - throw
        // 4b. else
        //        - restore old alarm handler
      }

      void gpi_api_t::stop ()
      {
      }

      void gpi_api_t::kill ()
      {
      }

      gpi::size_t gpi_api_t::number_of_counters () const;
      gpi::size_t gpi_api_t::number_of_queues () const;
      gpi::size_t gpi_api_t::queue_depth () const;
      gpi::version_t gpi_api_t::version () const;
      gpi::port_t gpi_api_t::port () const;
      gpi::size_t gpi_api_t::number_of_nodes () const;
      std::string gpi_api_t::hostname (const gpi::rank_t) const;
      gpi::rank_t gpi_api_t::rank () const;
      void * gpi_api_t::dma_ptr (void);

      void gpi_api_t::set_network_type (const gpi::network_type_t);
      void gpi_api_t::set_port (const gpi::port_t);
      void gpi_api_t::set_mtu (const gpi::size_t);
      void gpi_api_t::set_number_of_processes (const gpi::size_t);

      bool gpi_api_t::ping (const gpi::rank_t) const;
      bool gpi_api_t::ping (std::string const & hostname) const;
      bool gpi_api_t::is_master (void) const;
      bool gpi_api_t::is_slave (void) const;

      void gpi_api_t::barrier (void) const;

      void gpi_api_t::read_dma ( const offset_t local_offset
                               , const offset_t remote_offset
                               , const size_t amount
                               , const rank_t from_node
                               , const queue_desc_t queue
                               );
      void gpi_api_t::write_dma ( const offset_t local_offset
                                , const offset_t remote_offset
                                , const size_t amount
                                , const rank_t to_node
                                , const queue_desc_t queue
                                );

      void gpi_api_t::send_dma ( const offset_t local_offset
                               , const size_t amount
                               , const rank_t to_node
                               , const queue_desc_t queue
                               );
      void gpi_api_t::recv_dma ( const offset_t local_offset
                               , const size_t size
                               , const rank_t from_node
                               , const queue_desc_t queue
                               );

      size_t gpi_api_t::wait_dma (const queue_desc_t queue);

      void gpi_api_t::send_passive ( const offset_t local_offset
                                   , const size_t size
                                   , const rank_t to_rank
                                   );
      void gpi_api_t::recv_passive ( const offset_t local_offset
                                   , const size_t size
                                   , rank_t & from_rank
                                   );
      void gpi_api_t::wait_passive ( void );

      // ***** private functions
      void handle_alarm (int)
      {
        m_startup_done = false;
        throw exception::startup_failed ("timeout");
      }
    }
  }
}
