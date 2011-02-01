#include "gpi.hpp"

#include <csignal> // sigwait
#include <cstring> // strerror
#include <limits>

#include <GPI/GPI.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <fhglog/minimal.hpp>

namespace gpi
{
  namespace server
  {
    typedef boost::function<void (int)> signal_handler_t;

    gpi_api_t::gpi_api_t ()
      : m_ac (0)
      , m_av (NULL)
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
      // fire up a thread that handles signals


      // 1. register alarm signal handler
      /*
        signal_handler_t handler
        (boost::bind ( &gpi_api_t::handle_alarm
        , this
        , _1
        )
        );
      */

      alarm (timeout);

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

    gpi::size_t gpi_api_t::number_of_counters () const
    {
      return 0;
    }

    gpi::size_t gpi_api_t::number_of_queues () const
    {
      return 0;
    }

    gpi::size_t gpi_api_t::queue_depth () const
    {
      return 0;
    }

    gpi::version_t gpi_api_t::version () const
    {
      return 0;
    }

    gpi::port_t gpi_api_t::port () const
    {
      return 0;
    }

    gpi::size_t gpi_api_t::number_of_nodes () const
    {
      return 1;
    }

    std::string gpi_api_t::hostname (const gpi::rank_t) const
    {
      return "localhost";
    }

    gpi::rank_t gpi_api_t::rank () const
    {
      return 0;
    }

    void * gpi_api_t::dma_ptr (void)
    {
      return 0;
    }

    void gpi_api_t::set_network_type (const gpi::network_type_t)
    {
    }

    void gpi_api_t::set_port (const gpi::port_t)
    {
    }

    void gpi_api_t::set_mtu (const gpi::size_t)
    {
    }

    void gpi_api_t::set_number_of_processes (const gpi::size_t)
    {
    }

    bool gpi_api_t::ping (const gpi::rank_t) const
    {
      return false;
    }
    bool gpi_api_t::ping (std::string const & hostname) const
    {
      return false;
    }
    bool gpi_api_t::is_master (void) const
    {
      return true;
    }
    bool gpi_api_t::is_slave (void) const
    {
      return false;
    }

    void gpi_api_t::barrier (void) const
    {
    }

    void gpi_api_t::read_dma ( const offset_t local_offset
                             , const offset_t remote_offset
                             , const size_t amount
                             , const rank_t from_node
                             , const queue_desc_t queue
                             )
    {
    }
    void gpi_api_t::write_dma ( const offset_t local_offset
                              , const offset_t remote_offset
                              , const size_t amount
                              , const rank_t to_node
                              , const queue_desc_t queue
                              )
    {
    }

    void gpi_api_t::send_dma ( const offset_t local_offset
                             , const size_t amount
                             , const rank_t to_node
                             , const queue_desc_t queue
                             )
    {
    }
    void gpi_api_t::recv_dma ( const offset_t local_offset
                             , const size_t size
                             , const rank_t from_node
                             , const queue_desc_t queue
                             )
    {
    }

    size_t gpi_api_t::wait_dma (const queue_desc_t queue)
    {
      return 0;
    }

    void gpi_api_t::send_passive ( const offset_t local_offset
                                 , const size_t size
                                 , const rank_t to_rank
                                 )
    {
    }

    void gpi_api_t::recv_passive ( const offset_t local_offset
                                 , const size_t size
                                 , rank_t & from_rank
                                 )
    {
    }
    void gpi_api_t::wait_passive ( void )
    {
    }

    // ***** private functions
    void gpi_api_t::signal_handler ()
    {
      // signal handler thread
      char buf[1024];
      sigset_t restrict;
      sigfillset (&restrict);
      int sig (0);

      for (;;)
      {
        int ec = sigwait (&restrict, &sig);
        if (ec == 0)
        {
          handle_signal (sig);
        }
        else
        {
          LOG( ERROR
             , "sigwait() returned an error [" << ec << "]: " << strerror_r ( ec
                                                                            , buf
                                                                            , sizeof(buf)
                                                                            )
             );
        }
      }
    }

    void gpi_api_t::handle_signal (int sig)
    {
      switch (sig)
      {
      case SIGALRM:
        m_startup_done = false;
        break;
      };
    }
  }
}
