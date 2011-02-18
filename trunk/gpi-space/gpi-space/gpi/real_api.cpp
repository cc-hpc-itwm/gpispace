#include "api.hpp"

#include <csignal> // sigwait
#include <cstring> // strerror
#include <limits>

#include <GPI/GPI.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <fhglog/minimal.hpp>

#include <gpi-space/exception.hpp>
#include <gpi-space/signal_handler.hpp>

namespace gpi
{
  namespace api
  {
    typedef boost::function<void (int)> signal_handler_t;

    gpi_api_t * gpi_api_t::instance = 0;

    gpi_api_t & gpi_api_t::create (int ac, char *av[])
    {
      if (instance == 0)
      {
        instance = new gpi_api_t;
        instance->init (ac, av);
        return *instance;
      }
      else
      {
        throw std::runtime_error ("already created!");
      }
    }

    gpi_api_t & gpi_api_t::get ()
    {
      assert (instance);
      return *instance;
    }

    void gpi_api_t::destroy ()
    {
      if (instance)
      {
        delete instance;
        instance = 0;
      }
    }

    gpi_api_t::gpi_api_t ()
      : m_ac (0)
      , m_av (NULL)
      , m_is_master (true)
      , m_startup_done (false)
      , m_rank (std::numeric_limits<rank_t>::max())
      , m_num_nodes (0)
      , m_mem_size (0)
      , m_dma (0)
    { }

    gpi_api_t::~gpi_api_t ()
    {
      try
      {
        shutdown ();
      }
      catch (std::exception const & ex)
      {
        LOG(WARN, "could not stop gpi_api_t on rank " << rank());
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
      assert (! m_startup_done);

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

      if (timeout) alarm (timeout);
      startGPI (m_ac, m_av, "", m_mem_size);
      if (timeout) alarm (0);

      m_rank = getRankGPI();
      m_dma  = getDmaMemPtrGPI();
      m_num_nodes = generateHostlistGPI();
      m_startup_done = true;
    }

    void gpi_api_t::stop ()
    {
      if (m_startup_done)
      {
        shutdownGPI();
        m_startup_done = false;
      }
    }

    void gpi_api_t::kill ()
    {
      if (is_master())
      {
        if (0 != killProcsGPI())
        {
          throw gpi::exception::gpi_error
            (gpi::error::kill_procs_failed());
        }
      }
      else
      {
        throw gpi::exception::gpi_error
          ( gpi::error::operation_not_permitted()
          , "kill() is not allowed on a slave"
          );
      }
    }

    void gpi_api_t::shutdown ()
    {
      if (m_startup_done)
      {
        if (is_master ())
        {
          this->kill ();
        }
        else
        {
          stop ();
        }
        m_startup_done = false;
      }
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

    gpi::size_t gpi_api_t::memory_size () const
    {
      return m_mem_size;
    }

    gpi::size_t gpi_api_t::open_dma_requests (const queue_desc_t q) const
    {
      assert (m_startup_done);
      int rc (openDMARequestsGPI (q));
      if (rc < 0)
      {
        throw gpi::exception::gpi_error(gpi::error::open_dma_requests_failed());
      }
      return gpi::size_t (rc);
    }

    bool gpi_api_t::max_dma_requests_reached (const queue_desc_t q) const
    {
      assert (m_startup_done);
      return (open_dma_requests(q) >= queue_depth());
    }

    gpi::size_t gpi_api_t::open_passive_requests () const
    {
      assert (m_startup_done);
      int rc (openDMAPassiveRequestsGPI ());
      if (rc < 0)
      {
        throw gpi::exception::gpi_error(gpi::error::open_passive_requests_failed());
      }
      return gpi::size_t (rc);
    }

    bool gpi_api_t::max_passive_requests_reached (void) const
    {
      assert (m_startup_done);
      return (open_passive_requests() >= queue_depth());
    }

    std::string gpi_api_t::hostname (const gpi::rank_t r) const
    {
      // TODO: ap: cache the hostnames locally
      return getHostnameGPI (r);
    }

    gpi::rank_t gpi_api_t::rank () const
    {
      assert (m_startup_done);
      return m_rank;
    }

    gpi::error_vector_t gpi_api_t::get_error_vector (const gpi::queue_desc_t q) const
    {
      assert (m_startup_done);
      unsigned char *gpi_err_vec (getErrorVectorGPI(q));
      if (! gpi_err_vec)
      {
        throw gpi::exception::gpi_error
          (gpi::error::get_error_vector_failed());
      }

      gpi::error_vector_t v (number_of_nodes());
      for (std::size_t i (0); i < number_of_nodes(); ++i)
      {
        v.set (i, (gpi_err_vec[i] == 1));
      }
      return v;
    }

    void * gpi_api_t::dma_ptr (void)
    {
      assert (m_startup_done);
      return m_dma;
    }

    void gpi_api_t::set_network_type (const gpi::network_type_t n)
    {
      int rc (setNetworkGPI (GPI_NETWORK_TYPE(n)));
      if (rc != 0)
        throw gpi::exception::gpi_error(gpi::error::set_network_type_failed());
    }

    void gpi_api_t::set_port (const gpi::port_t p)
    {
      int rc (setPortGPI (p));
      if (rc != 0)
        throw gpi::exception::gpi_error(gpi::error::set_port_failed());
    }

    void gpi_api_t::set_mtu (const gpi::size_t mtu)
    {
      int rc (setMtuSizeGPI (mtu));
      if (rc != 0)
        throw gpi::exception::gpi_error(gpi::error::set_mtu_failed());
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

    void gpi_api_t::check (const gpi::rank_t node) const
    {
      if (! is_master())
      {
        throw gpi::exception::gpi_error
          ( gpi::error::operation_not_permitted()
          , "check(rank) is not allowed on a slave"
          );
      }

      std::string host (hostname (node));

      LOG(DEBUG, "checking GPI on host := " << host << " rank := " << node);

      if (!ping (host))
      {
        LOG(ERROR, "failed to ping GPI daemon on host := " << host);
        throw gpi::exception::gpi_error
          ( gpi::error::ping_check_failed () );
      }

      int rc (0);
      int errors (0);

      rc = checkPortGPI (host.c_str(), port());
      if (rc != 0)
      {
        LOG(WARN, "failed to check port " << port() << " on " << host << " with error " << rc);
        ++errors;

        if (findProcGPI (host.c_str()) == 0)
        {
          LOG(WARN, "another GPI binary is still running and blocking the port, trying to kill it now");
          if (killProcsGPI() == 0)
          {
            LOG(INFO, "successfully killed old processes");
            --errors;
          }
          else
          {
            LOG(ERROR, "could not kill old processes");
          }
        }
        else
        {
          LOG(ERROR, "another program seems to block port " << port());
        }
      }

      rc = checkSharedLibsGPI(host.c_str());
      if (rc != 0)
      {
        LOG(ERROR, "shared library requirements could not be met on host " << host << " with error " << rc);
        throw gpi::exception::gpi_error
          ( gpi::error::libs_check_failed () );
      }

      rc = runIBTestGPI (host.c_str());
      if (rc != 0)
      {
        LOG(ERROR, "InfiniBand test failed for host " << host << " rank " << node << " with error " << rc);
        throw gpi::exception::gpi_error
          ( gpi::error::ib_check_failed () );
      }
    }

    void gpi_api_t::check (void) const
    {
      if (!is_master())
      {
        throw gpi::exception::gpi_error
          ( gpi::error::operation_not_permitted()
          , "check() is not allowed on a slave"
          );
      }

      LOG(DEBUG, "running GPI check...");
      for (rank_t nd (0); nd < number_of_nodes(); ++nd)
      {
        check (nd);
      }
      LOG(INFO, "GPI check successful.");
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
      assert (m_startup_done);
      barrierGPI();
    }

    void gpi_api_t::lock (void) const
    {
      assert (m_startup_done);
      int rc (globalResourceLockGPI());
      if (rc != 0)
      {
        throw gpi::exception::gpi_error
          (gpi::error::global_lock_failed());
      }
    }

    void gpi_api_t::unlock (void) const
    {
      assert (m_startup_done);
      int rc (globalResourceUnlockGPI());
      if (rc != 0)
      {
        throw gpi::exception::gpi_error
          (gpi::error::global_unlock_failed());
      }
    }

    void gpi_api_t::read_dma ( const offset_t local_offset
                             , const offset_t remote_offset
                             , const size_t amount
                             , const rank_t from_node
                             , const queue_desc_t queue
                             )
    {
      assert (m_startup_done);
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
      assert (m_startup_done);
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
      assert (m_startup_done);
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
      assert (m_startup_done);
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
      assert (m_startup_done);
      int rc
        (waitDmaGPI(queue));
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
      assert (m_startup_done);
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
      assert (m_startup_done);
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
      assert (m_startup_done);
      int rc(waitDmaPassiveGPI());
      if (rc < 0)
      {
        throw gpi::exception::gpi_error
          (gpi::error::wait_passive_failed());
      }
      return size_t (rc);
    }

    // private functions
    int gpi_api_t::startup_timedout_cb (int)
    {
      m_startup_done = false;
      // currently there is no other way than to exit :-(
      this->kill ();
      exit (gpi::error::errc::startup_failed);
    }
  }
}
