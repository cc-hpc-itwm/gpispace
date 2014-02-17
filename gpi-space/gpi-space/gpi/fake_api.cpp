#include "fake_api.hpp"

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhg/assert.hpp>

#include <gpi-space/exception.hpp>

#include "system.hpp"

namespace gpi
{
  namespace api
  {
    fake_gpi_api_t::fake_gpi_api_t (bool is_master)
      : m_is_master (is_master)
      , m_binary_path("")
      , m_startup_done (false)
      , m_rank (0)
      , m_mem_size (0)
      , m_dma (0)
      , m_queue_count (8)
    {
      m_dma_request_count.assign (m_queue_count, 0);
    }

    fake_gpi_api_t::~fake_gpi_api_t ()
    {
      shutdown ();
    }

    void fake_gpi_api_t::set_memory_size (const gpi::size_t sz)
    {
      m_mem_size = sz;
    }

    // wrapped C function calls
    void fake_gpi_api_t::set_binary_path (const char *path)
    {
      m_binary_path = path;
    }

    void fake_gpi_api_t::start (int, char **, const gpi::timeout_t)
    {
      assert (! m_startup_done);
      if (m_dma)
        free (m_dma);

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

      m_dma = malloc(m_mem_size * sizeof (char));
      if (! m_dma)
      {
        throw gpi::exception::gpi_error
          ( gpi::error::startup_failed()
          , "could not allocate memory"
          );
      }
      memset (m_dma, 0, m_mem_size);

      m_startup_done = true;
    }

    void fake_gpi_api_t::stop ()
    {
      lock_type lock (m_mutex);
      if (m_startup_done)
      {
        if (m_dma)
        {
          free (m_dma); m_dma = 0;
        }
        m_startup_done = false;
      }
    }

    void fake_gpi_api_t::kill ()
    {}

    void fake_gpi_api_t::shutdown ()
    {
      stop ();
    }

    gpi::size_t fake_gpi_api_t::number_of_queues () const
    {
      return m_queue_count;
    }

    gpi::size_t fake_gpi_api_t::queue_depth () const
    {
      return 1024;
    }

    gpi::size_t fake_gpi_api_t::number_of_counters () const
    {
      return 0;
    }

    gpi::version_t fake_gpi_api_t::version () const
    {
      return 0.01;
    }

    gpi::port_t fake_gpi_api_t::port () const
    {
      return 42423;
    }

    gpi::size_t fake_gpi_api_t::number_of_nodes () const
    {
      return 1;
    }

    gpi::size_t fake_gpi_api_t::memory_size () const
    {
      return m_mem_size;
    }

    gpi::size_t fake_gpi_api_t::open_dma_requests (const queue_desc_t q) const
    {
      lock_type lock (m_mutex);
      return m_dma_request_count[q];
    }

    bool fake_gpi_api_t::max_dma_requests_reached (const queue_desc_t q) const
    {
      return (open_dma_requests(q) >= queue_depth());
    }

    gpi::size_t fake_gpi_api_t::open_passive_requests () const
    {
      return 0;
    }

    bool fake_gpi_api_t::max_passive_requests_reached (void) const
    {
      return (open_passive_requests() >= queue_depth());
    }

    const char * fake_gpi_api_t::hostname (const gpi::rank_t) const
    {
      return "localhost";
    }

    gpi::rank_t fake_gpi_api_t::rank () const
    {
      return m_rank;
    }

    gpi::error_vector_t fake_gpi_api_t::get_error_vector (const gpi::queue_desc_t) const
    {
      gpi::error_vector_t v (number_of_nodes());
      for (std::size_t i (0); i < number_of_nodes(); ++i)
      {
        v.set (i, 0);
      }
      return v;
    }

    void * fake_gpi_api_t::dma_ptr (void)
    {
      assert (m_startup_done);
      return m_dma;
    }

    void fake_gpi_api_t::set_network_type (const gpi::network_type_t)
    {}

    void fake_gpi_api_t::set_port (const gpi::port_t)
    {}

    void fake_gpi_api_t::set_mtu (const gpi::size_t)
    {}

    void fake_gpi_api_t::set_number_of_processes (const gpi::size_t)
    {}

    bool fake_gpi_api_t::ping (const gpi::rank_t r) const
    {
      return ping (hostname (r));
    }

    bool fake_gpi_api_t::ping (const char *) const
    {
      return true;
    }

    int fake_gpi_api_t::check (const gpi::rank_t node) const
    {
      return check (hostname (node));
    }

    int fake_gpi_api_t::check (const char * host) const
    {
      if (ping(host)) return 0;
      else            return 1;
    }

    int fake_gpi_api_t::check (void) const
    {
      lock_type lock (m_mutex);
      if (!is_master())
      {
        throw gpi::exception::gpi_error
          ( gpi::error::operation_not_permitted()
          , "check() is not allowed on a slave"
          );
      }

      int ec = 0;
      for (rank_t nd (0); nd < number_of_nodes(); ++nd)
      {
        ec += check (nd);
      }
      if (ec)
      {
        LOG(WARN, "gpi check failed!");
      }
      else
      {
        LOG(INFO, "GPI check successful.");
      }
      return ec;
    }

    bool fake_gpi_api_t::is_master (void) const
    {
      return m_is_master;
    }
    bool fake_gpi_api_t::is_slave (void) const
    {
      return !is_master();
    }

    void fake_gpi_api_t::barrier (void) const
    {}

    bool fake_gpi_api_t::try_lock (void) const
    {
      return true;
    }

    void fake_gpi_api_t::lock (void) const
    {
      {
        lock_type lock (m_mutex);
        if (! m_startup_done)
        {
          throw gpi::exception::gpi_error
            (gpi::error::operation_not_permitted("lock: gpi not started"));
        }
      }
    }

    void fake_gpi_api_t::unlock (void) const
    {
      {
        lock_type lock (m_mutex);
        if (! m_startup_done)
        {
          throw gpi::exception::gpi_error
            (gpi::error::operation_not_permitted("unlock: gpi not started"));
        }
      }
    }

    void fake_gpi_api_t::read_dma ( const offset_t local_offset
                                  , const offset_t remote_offset
                                  , const size_t amount
                                  , const rank_t from_node
                                  , const queue_desc_t queue
                                  )
    {
      lock_type lock (m_mutex);

      assert (m_startup_done);

      if (from_node != 0)
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
      else
      {
        if (m_dma_request_count[queue]+1 > queue_depth())
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
        else
        {
          ++m_dma_request_count[queue];
        }
        memcpy ( (char*)m_dma + local_offset
               , (char*)m_dma + remote_offset
               , amount
               );
      }
    }
    void fake_gpi_api_t::write_dma ( const offset_t local_offset
                                   , const offset_t remote_offset
                                   , const size_t amount
                                   , const rank_t to_node
                                   , const queue_desc_t queue
                                   )
    {
      lock_type lock (m_mutex);

      assert (m_startup_done);

      if (to_node != 0)
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
      else
      {
        if (m_dma_request_count[queue]+1 > queue_depth())
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
        else
        {
          ++m_dma_request_count[queue];
        }

        memcpy ( (char*)m_dma + remote_offset
               , (char*)m_dma + local_offset
               , amount
               );
      }
    }

    void fake_gpi_api_t::send_dma ( const offset_t local_offset
                                  , const size_t amount
                                  , const rank_t to_node
                                  , const queue_desc_t queue
                                  )
    {
      lock_type lock (m_mutex);
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

    void fake_gpi_api_t::recv_dma ( const offset_t local_offset
                                  , const size_t amount
                                  , const rank_t from_node
                                  , const queue_desc_t queue
                                  )
    {
      lock_type lock (m_mutex);
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

    size_t fake_gpi_api_t::wait_dma (const queue_desc_t queue)
    {
      assert (m_startup_done);

      size_t cnt (m_dma_request_count[queue]);
      m_dma_request_count[queue] = 0;
      return cnt;
    }

    void fake_gpi_api_t::send_passive ( const offset_t local_offset
                                      , const size_t amount
                                      , const rank_t to_node
                                      )
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

    void fake_gpi_api_t::recv_passive ( const offset_t local_offset
                                 , const size_t amount
                                 , rank_t & from_node
                                 )
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

    size_t fake_gpi_api_t::wait_passive ( void )
    {
      return 0;
    }
  }
}
