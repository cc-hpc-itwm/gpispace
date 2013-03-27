#include "real_api.hpp"

#include <csignal> // sigwait
#include <cstring> // strerror
#include <limits>

#include <GPI.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <fhglog/minimal.hpp>
#include <fhg/assert.hpp>

#include <gpi-space/exception.hpp>
#include "system.hpp"

namespace gpi
{
  namespace api
  {
    real_gpi_api_t::real_gpi_api_t ()
      : m_is_master (true)
      , m_binary_path ("")
      , m_startup_done (false)
      , m_rank (std::numeric_limits<rank_t>::max())
      , m_mem_size (0)
      , m_dma (0)
    { }

    real_gpi_api_t::~real_gpi_api_t ()
    {
      try
      {
        shutdown ();
      }
      catch (std::exception const & ex)
      {
        LOG(WARN, "could not stop real_gpi_api_t on rank " << rank());
      }
    }

    void real_gpi_api_t::set_memory_size (const gpi::size_t sz)
    {
      m_mem_size = sz;
    }

    int real_gpi_api_t::build_hostlist ()
    {
      return generateHostlistGPI();
    }

    // wrapped C function calls
    void real_gpi_api_t::set_binary_path (const char *path)
    {
      m_binary_path = path;
    }

    void real_gpi_api_t::start (int ac, char *av[], const gpi::timeout_t timeout)
    {
      assert (! m_startup_done);

      if (sys::get_total_memory_size() < m_mem_size)
      {
        LOG( ERROR
           , "requested memory size (" << m_mem_size << ")"
           <<" exceeds total memory size (" << sys::get_total_memory_size() << ")"
           );
        throw gpi::exception::gpi_error
          ( gpi::error::startup_failed()
          , "not enough memory"
          );
      }
      else if (sys::get_avail_memory_size() < m_mem_size)
      {
        LOG( WARN
           , "requested memory size (" << m_mem_size << ")"
           <<" exceeds available memory size (" << sys::get_avail_memory_size() << ")"
           );
      }

      int rc (0);

      if (timeout) alarm (timeout);
      if (is_master())
      {
        rc = startGPI (1, av, m_binary_path, m_mem_size);
      }
      else
      {
        rc = startGPI (ac, av, m_binary_path, m_mem_size);
      }
      if (timeout) alarm (0);

      if (rc != 0)
      {
        throw gpi::exception::gpi_error
          ( gpi::error::startup_failed()
          , "startGPI() failed"
          );
      }

      m_rank = getRankGPI();
      m_dma  = getDmaMemPtrGPI();
      m_startup_done = true;
    }

    void real_gpi_api_t::stop ()
    {
      lock_type lock (m_mutex);
      if (m_startup_done)
      {
        shutdownGPI();
        m_startup_done = false;
      }
    }

    void real_gpi_api_t::kill ()
    {
      killProcsGPI();
    }

    void real_gpi_api_t::shutdown ()
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

    gpi::size_t real_gpi_api_t::number_of_queues () const
    {
      return getNumberOfQueuesGPI();
    }

    gpi::size_t real_gpi_api_t::queue_depth () const
    {
      return getQueueDepthGPI();
    }

    gpi::size_t real_gpi_api_t::number_of_counters () const
    {
      return getNumberOfCountersGPI();
    }

    gpi::version_t real_gpi_api_t::version () const
    {
      return getVersionGPI();
    }

    gpi::port_t real_gpi_api_t::port () const
    {
      return getPortGPI();
    }

    gpi::size_t real_gpi_api_t::number_of_nodes () const
    {
      return getNodeCountGPI();
    }

    gpi::size_t real_gpi_api_t::memory_size () const
    {
      return m_mem_size;
    }

    gpi::size_t real_gpi_api_t::max_transfer_size () const
    {
      return (1 << 30);
    }

    gpi::size_t real_gpi_api_t::open_dma_requests (const queue_desc_t q) const
    {
      assert (m_startup_done);
      int rc (openDmaRequestsGPI (q));
      if (rc < 0)
      {
        throw gpi::exception::gpi_error(gpi::error::open_dma_requests_failed());
      }
      return gpi::size_t (rc);
    }

    bool real_gpi_api_t::max_dma_requests_reached (const queue_desc_t q) const
    {
      return (open_dma_requests(q) >= queue_depth());
    }

    gpi::size_t real_gpi_api_t::open_passive_requests () const
    {
      assert (m_startup_done);
      int rc (openDmaPassiveRequestsGPI ());
      if (rc < 0)
      {
        throw gpi::exception::gpi_error(gpi::error::open_passive_requests_failed());
      }
      return gpi::size_t (rc);
    }

    bool real_gpi_api_t::max_passive_requests_reached (void) const
    {
      return (open_passive_requests() >= queue_depth());
    }

    const char * real_gpi_api_t::hostname (const gpi::rank_t r) const
    {
      return getHostnameGPI (r);
    }

    gpi::rank_t real_gpi_api_t::rank () const
    {
      assert (m_startup_done);
      return m_rank;
    }

    gpi::error_vector_t real_gpi_api_t::get_error_vector (const gpi::queue_desc_t q) const
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
        v.set (i, (gpi_err_vec[i] != 0));
      }
      return v;
    }

    void * real_gpi_api_t::dma_ptr (void)
    {
      assert (m_startup_done);
      return m_dma;
    }

    void real_gpi_api_t::set_network_type (const gpi::network_type_t n)
    {
      int rc (setNetworkGPI (GPI_NETWORK_TYPE(n)));
      if (rc != 0)
        throw gpi::exception::gpi_error(gpi::error::set_network_type_failed());
    }

    void real_gpi_api_t::set_port (const gpi::port_t p)
    {
      int rc (setPortGPI (p));
      if (rc != 0)
        throw gpi::exception::gpi_error(gpi::error::set_port_failed());
    }

    void real_gpi_api_t::set_mtu (const gpi::size_t mtu)
    {
      int rc (setMtuSizeGPI (mtu));
      if (rc != 0)
        throw gpi::exception::gpi_error(gpi::error::set_mtu_failed());
    }

    void real_gpi_api_t::set_number_of_processes (const gpi::size_t n)
    {
      setNpGPI (n);
    }

    bool real_gpi_api_t::ping (const gpi::rank_t rank) const
    {
      return ping (hostname (rank));
    }
    bool real_gpi_api_t::ping (const char * hostname) const
    {
      return (pingDaemonGPI(hostname) == 0);
    }

    void real_gpi_api_t::clear_caches ()
    {
      if (! is_master())
      {
        throw gpi::exception::gpi_error
          ( gpi::error::operation_not_permitted()
          , "clear_caches is not allowed on a slave"
          );
      }

      LOG(DEBUG, "clearing file caches...");
      int node_count = generateHostlistGPI();
      for (int i = 0 ; i < node_count ; ++i)
      {
        const char * hostname = getHostnameGPI(i);
        int error_code = clearFileCacheGPI(hostname);
        if (1 == error_code)
        {
          LOG(DEBUG, hostname << " - ok");
        }
        else if (-42 == error_code)
        {
          LOG(WARN, "clear cache on " << hostname << " timed out");
        }
        else
        {
          LOG(WARN, "clear cache on " << hostname << " failed: " << error_code);
        }
      }
    }

    int real_gpi_api_t::check (const gpi::rank_t node) const
    {
      if (! is_master())
      {
        throw gpi::exception::gpi_error
          ( gpi::error::operation_not_permitted()
          , "check(rank) is not allowed on a slave"
          );
      }

      const char * host (hostname (node));
      return check (host);
    }


    int real_gpi_api_t::check (const char *host) const
    {
      LOG(DEBUG, "checking GPI on host := " << host << " port := " << port());

      if (!ping (host))
      {
        LOG(ERROR, "failed to ping GPI daemon on host := " << host);
        throw gpi::exception::gpi_error
          ( gpi::error::ping_check_failed ()
          , "host = " + std::string(host)
          );
      }

      int rc (0);

      rc = findProcGPI (host);
      if (rc == 0)
      {
        throw gpi::exception::gpi_error
          ( gpi::error::another_binary_running()
          , "[" + std::string(host) + "]:" + boost::lexical_cast<std::string>(port())
          );
      }

      rc = checkPortGPI(host, port());
      if (rc != 0)
      {
        throw gpi::exception::gpi_error
          ( gpi::error::port_check_failed()
          , "[" + std::string(host) + "]:" + boost::lexical_cast<std::string>(port())
          );
      }

      rc = checkSharedLibsGPI(host);
      if (rc != 0)
      {
        throw gpi::exception::gpi_error
          ( gpi::error::libs_check_failed ()
          , "[" + std::string(host) + "]:" + boost::lexical_cast<std::string>(port())
          );
      }

      rc = runIBTestGPI (host);
      if (rc != 0)
      {
        throw gpi::exception::gpi_error
          ( gpi::error::ib_check_failed ()
          , "[" + std::string(host) + "]:" + boost::lexical_cast<std::string>(port())
          );
      }

      return rc;
    }

    int real_gpi_api_t::check (void) const
    {
      if (!is_master())
      {
        throw gpi::exception::gpi_error
          ( gpi::error::operation_not_permitted()
          , "check() is not allowed on a slave"
          );
      }

      size_t max_rank = 0;
      {
        int tmp = generateHostlistGPI();
        if (tmp <= 0)
        {
          throw gpi::exception::gpi_error
            ( gpi::error::internal_error()
            , "generateHostlist() failed"
            );
        }
        max_rank  = (size_t)(tmp);
      }

      LOG( TRACE, "running GPI check...");
      int rc (0);
      for (rank_t nd (0); nd < max_rank; ++nd)
      {
        try
        {
          check (nd);
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "check on host " << hostname(nd) << " failed: " << ex.what());
          rc += 1;
        }
      }

      if (rc)
      {
        LOG(ERROR, "GPI check failed on " << rc << " hosts");
        throw gpi::exception::gpi_error
          ( gpi::error::unknown()
          , "GPI check failed"
          );
      }

      return 0;
    }

    void real_gpi_api_t::set_is_master(const bool b)
    {
      m_is_master = b;
    }

    bool real_gpi_api_t::is_master (void) const
    {
      return m_is_master;
    }
    bool real_gpi_api_t::is_slave (void) const
    {
      return !is_master();
    }

    void real_gpi_api_t::barrier (void) const
    {
      lock_type lock (m_mutex);

      assert (m_startup_done);
      barrierGPI();
    }

    bool real_gpi_api_t::try_lock (void) const
    {
      lock_type lock (m_mutex);
      assert (m_startup_done);

      return 0 == globalResourceLockGPI();
    }

    void real_gpi_api_t::lock (void) const
    {
      while (globalResourceLockGPI() != 0)
        usleep (m_rank * (rand() % 100000));
    }

    void real_gpi_api_t::unlock (void) const
    {
      int rc (globalResourceUnlockGPI());
      if (rc != 0)
      {
        throw gpi::exception::gpi_error
          (gpi::error::global_unlock_failed("not owner"));
      }
    }

    void real_gpi_api_t::read_dma ( const offset_t local_offset
                                  , const offset_t remote_offset
                                  , const size_t amount
                                  , const rank_t from_node
                                  , const queue_desc_t queue
                                  )
    {
      assert (m_startup_done);

      size_t remaining (amount);
      const size_t chunk_size (max_transfer_size ());

      size_t l_off (local_offset);
      size_t r_off (remote_offset);

      while (remaining)
      {
        const size_t to_transfer (std::min (chunk_size, remaining));

        if (max_dma_requests_reached (queue))
        {
          wait_dma (queue);
        }

        DLOG( TRACE
            , "real_api: readDMA:"
            << " remote: " << r_off
            << " local: " << l_off
            << " amount: " << to_transfer
            << " node: " << from_node
            << " queue: " << queue
            );

        int rc
          (readDmaGPI ( l_off
                      , r_off
                      , to_transfer
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

        remaining -= to_transfer;
        l_off     += to_transfer;
        r_off     += to_transfer;
      }
    }

    void real_gpi_api_t::write_dma ( const offset_t local_offset
                              , const offset_t remote_offset
                              , const size_t amount
                              , const rank_t to_node
                              , const queue_desc_t queue
                              )
    {
      assert (m_startup_done);

      size_t remaining (amount);
      const size_t chunk_size (max_transfer_size ());

      size_t l_off (local_offset);
      size_t r_off (remote_offset);

      while (remaining)
      {
        const size_t to_transfer (std::min (chunk_size, remaining));

        if (max_dma_requests_reached (queue))
        {
          wait_dma (queue);
        }

        DLOG( TRACE
            , "real_api: writeDMA:"
            << " remote: " << r_off
            << " local: " << l_off
            << " amount: " << to_transfer
            << " node: " << to_node
            << " queue: " << queue
            );

        int rc
          (writeDmaGPI ( l_off
                       , r_off
                       , to_transfer
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

        remaining -= to_transfer;
        l_off     += to_transfer;
        r_off     += to_transfer;
      }
    }

    void real_gpi_api_t::send_dma ( const offset_t local_offset
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
    void real_gpi_api_t::recv_dma ( const offset_t local_offset
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

    size_t real_gpi_api_t::wait_dma (const queue_desc_t queue)
    {
      assert (m_startup_done);
      int rc
        (waitDma2GPI(queue));
      if (rc < 0)
      {
        MLOG ( ERROR
             , "waitDMA failed on queue " << queue << ": " << rc
             << " ev := " << get_error_vector (queue)
             );

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

    void real_gpi_api_t::send_passive ( const offset_t local_offset
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

    void real_gpi_api_t::recv_passive ( const offset_t local_offset
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

    size_t real_gpi_api_t::wait_passive ( void )
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
    int real_gpi_api_t::startup_timedout_cb (const gpi::timeout_t timeout, int)
    {
      m_startup_done = false;
      // currently there is no other way than to exit :-(
      LOG( ERROR
         , "Sorry, the GPI startup did not complete within " <<  timeout
         << " seconds. There is nothing I can do for you."
         );
      try
      {
        this->kill ();
      }
      catch (std::exception const &)
      {
      }
      exit (gpi::error::errc::startup_failed);
    }
  }
}
