#include "gpi.hpp"

#include <csignal> // sigwait
#include <cstring> // strerror
#include <limits>

#include <GPI/GPI.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <fhglog/minimal.hpp>

#include <gpi-space/signal_handler.hpp>

namespace gpi
{
  namespace server
  {
    typedef boost::function<void (int)> signal_handler_t;

    gpi_api_t::gpi_api_t ()
      : m_ac (0)
      , m_av (NULL)
      , m_is_master (true)
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
      m_is_master = isMasterProcGPI(m_ac, m_av);
      m_num_nodes = generateHostlistGPI();
    }

    void gpi_api_t::set_memory_size (const gpi::size_t sz)
    {
      m_mem_size = sz;
    }

    // wrapped C function calls
    void gpi_api_t::start (const gpi::timeout_t timeout)
    {
      // 1. register alarm signal handler
      gpi::signal::handler_t::scoped_connection_t
        conn (gpi::signal::handler().connect
             ( SIGALRM
             , boost::bind ( &gpi_api_t::startup_timedout_cb
                           , this
                           , _1
                           )
             )
             );

      m_startup_done = false;
      alarm (timeout);
      startGPI (m_ac, m_av, "", m_mem_size);
      alarm (0);
      m_startup_done = true;
      m_rank = getRankGPI();
    }

    void gpi_api_t::stop ()
    {
      throw exception::gpi_error
        (gpi::error::operation_not_implemented());
    }

    void gpi_api_t::kill ()
    {
      throw exception::gpi_error
        (gpi::error::operation_not_implemented());
    }

    gpi::size_t gpi_api_t::number_of_queues () const
    {
      return getNumberOfQueuesGPI();
    }

    gpi::size_t gpi_api_t::queue_depth () const
    {
      return getQueueDepthGPI();
    }

    gpi::size_t gpi_api_t::number_of_counters () const
    {
      return getNumberOfCountersGPI();
    }

    gpi::version_t gpi_api_t::version () const
    {
      return getVersionGPI();
    }

    gpi::port_t gpi_api_t::port () const
    {
      return getPortGPI();
    }

    gpi::size_t gpi_api_t::number_of_nodes () const
    {
      return m_num_nodes;
    }

    std::string gpi_api_t::hostname (const gpi::rank_t r) const
    {
      // TODO: ap: cache the hostnames locally
      return getHostnameGPI (r);
    }

    gpi::rank_t gpi_api_t::rank () const
    {
      return m_rank;
    }

    void * gpi_api_t::dma_ptr (void)
    {
      return getDmaMemPtrGPI();
    }

    void gpi_api_t::set_network_type (const gpi::network_type_t n)
    {
      int rc (setNetworkGPI (GPI_NETWORK_TYPE(n)));
      if (rc != 0)
        throw exception::gpi_error(gpi::error::set_network_type_failed());
    }

    void gpi_api_t::set_port (const gpi::port_t p)
    {
      int rc (setPortGPI (p));
      if (rc != 0)
        throw exception::gpi_error(gpi::error::set_port_failed());
    }

    void gpi_api_t::set_mtu (const gpi::size_t mtu)
    {
      int rc (setMtuSizeGPI (mtu));
      if (rc != 0)
        throw exception::gpi_error(gpi::error::set_mtu_failed());
    }

    void gpi_api_t::set_number_of_processes (const gpi::size_t n)
    {
      setNpGPI (n);
    }

    bool gpi_api_t::ping (const gpi::rank_t rank) const
    {
      return ping (hostname (rank));
    }
    bool gpi_api_t::ping (std::string const & hostname) const
    {
      return (pingDaemonGPI(hostname.c_str()) == 0);
    }
    bool gpi_api_t::is_master (void) const
    {
      return m_is_master;
    }
    bool gpi_api_t::is_slave (void) const
    {
      return !is_master();
    }

    void gpi_api_t::barrier (void) const
    {
      barrierGPI();
    }

    void gpi_api_t::read_dma ( const offset_t local_offset
                             , const offset_t remote_offset
                             , const size_t amount
                             , const rank_t from_node
                             , const queue_desc_t queue
                             )
    {
      int rc
        (readDmaGPI ( local_offset
                    , remote_offset
                    , amount
                    , from_node
                    , queue
                    )
        );

      if (rc != 0)
      {
        throw exception::dma_error
          ( gpi::error::read_dma_failed()
          , local_offset
          , remote_offset
          , from_node
          , rank()
          , amount
          , queue
          );
      }
    }
    void gpi_api_t::write_dma ( const offset_t local_offset
                              , const offset_t remote_offset
                              , const size_t amount
                              , const rank_t to_node
                              , const queue_desc_t queue
                              )
    {
      int rc
        (writeDmaGPI ( local_offset
                     , remote_offset
                     , amount
                     , to_node
                     , queue
                     )
        );

      if (rc != 0)
      {
        throw exception::dma_error
          ( gpi::error::write_dma_failed()
          , local_offset
          , remote_offset
          , rank()
          , to_node
          , amount
          , queue
          );
      }
    }

    void gpi_api_t::send_dma ( const offset_t local_offset
                             , const size_t amount
                             , const rank_t to_node
                             , const queue_desc_t queue
                             )
    {
      int rc
        (sendDmaGPI ( local_offset
                    , amount
                    , to_node
                    , queue
                    )
        );

      if (rc != 0)
      {
        throw exception::dma_error
          ( gpi::error::send_dma_failed()
          , local_offset
          , 0
          , rank()
          , to_node
          , amount
          , queue
          );
      }
    }
    void gpi_api_t::recv_dma ( const offset_t local_offset
                             , const size_t amount
                             , const rank_t from_node
                             , const queue_desc_t queue
                             )
    {
      int rc
        (recvDmaGPI ( local_offset
                    , amount
                    , from_node
                    , queue
                    )
        );

      if (rc != 0)
      {
        throw exception::dma_error
          ( gpi::error::recv_dma_failed()
          , local_offset
          , 0
          , from_node
          , rank()
          , amount
          , queue
          );
      }
    }

    size_t gpi_api_t::wait_dma (const queue_desc_t queue)
    {
      int rc
        (wait_dma (queue));
      if (rc < 0)
      {
        throw exception::dma_error
          ( gpi::error::wait_dma_failed()
          , 0
          , 0
          , rank ()
          , 0
          , 0
          , queue
          );
      }
      else
      {
        return size_t (rc);
      }
    }

    void gpi_api_t::send_passive ( const offset_t local_offset
                                 , const size_t amount
                                 , const rank_t to_node
                                 )
    {
      int rc
        (sendDmaPassiveGPI ( local_offset
                           , amount
                           , to_node
                           )
        );
      if (rc != 0)
      {
        throw exception::dma_error
          ( gpi::error::send_passive_failed()
          , local_offset
          , 0
          , rank()
          , to_node
          , amount
          , 0
          );
      }
    }

    void gpi_api_t::recv_passive ( const offset_t local_offset
                                 , const size_t amount
                                 , rank_t & from_node
                                 )
    {
      int tmp;
      int rc
        (recvDmaPassiveGPI ( local_offset
                           , amount
                           , &tmp
                           )
        );
      from_node = tmp;

      if (rc != 0)
      {
        throw exception::dma_error
          ( gpi::error::recv_passive_failed()
          , local_offset
          , 0
          , from_node
          , rank ()
          , amount
          , 0
          );
      }
    }

    size_t gpi_api_t::wait_passive ( void )
    {
      int rc(waitDmaPassiveGPI());
      if (rc < 0)
      {
        throw exception::gpi_error
          (gpi::error::wait_passive_failed());
      }
      return size_t (rc);
    }

    // private functions
    int gpi_api_t::startup_timedout_cb (int)
    {
      m_startup_done = false;
      // currently there is no other way than to exit :-(
      exit (gpi::error::errc::startup_failed);
    }
  }
}
