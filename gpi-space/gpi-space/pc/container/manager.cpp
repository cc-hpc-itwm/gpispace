/* -*- mode: c++ -*- */

#include "manager.hpp"

#include <boost/bind.hpp>

#include <fhglog/minimal.hpp>

#include <gpi-space/gpi/api.hpp>
#include <gpi-space/pc/global/topology.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/memory/manager.hpp>

#include <gpi-space/pc/memory/factory.hpp>

// TODO remove this - currently required for register_memory
#include <fhg/util/url.hpp>
#include <fhg/util/url_io.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      manager_t::manager_t (std::string const & p, std::string const &mem_url)
       : m_state (ST_STOPPED)
       , m_connector (*this, p)
       , m_process_counter (0)
       , m_default_memory_url (mem_url)
      {}

      manager_t::~manager_t ()
      {
        try
        {
          stop();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "error within dtor of container manager: " << ex.what());
        }
      }

      void manager_t::start ()
      {
        set_state (ST_STARTING);

        try
        {
          lock_type lock (m_mutex);
          initialize_topology ();
          initialize_memory_manager ();

          if (global::topology().is_master ())
          {
            MLOG (TRACE, "telling slaves to GO");
            global::topology ().go ();
          }
          else
          {
            MLOG (TRACE, "waiting for master to say GO");
            global::topology ().wait_for_go ();
          }

          m_connector.start ();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "manager could not be started: " << ex.what());
          set_state (ST_STOPPED);
          throw;
        }

        set_state (ST_STARTED);
      }

      void manager_t::stop ()
      {
        set_state (ST_STOPPING);

        {
          lock_type lock (m_mutex);
          m_connector.stop ();
          while (! m_processes.empty())
          {
            detach_process (m_processes.begin()->first);
          }

          global::memory_manager().clear();
          shutdown_topology ();
        }

        garbage_collect();

        set_state (ST_STOPPED);
      }

      manager_t::state_t manager_t::get_state () const
      {
        lock_type lcok (m_state_mutex);
        return m_state;
      }

      void manager_t::require_state (const state_t s) const
      {
        if (get_state () != s)
        {
          throw std::runtime_error ("state error: manager is in wrong state");
        }
      }

      void manager_t::set_state (const state_t new_state)
      {
        static bool table [NUM_STATES][NUM_STATES] =
          {
       // STOPPED   STARTING   STARTED  STOPPING
            {1,        1,         0,       1}, // STOPPED
            {1,        0,         1,       1}, // STARTING
            {0,        0,         0,       1}, // STARTED
            {1,        0,         0,       0}, // STOPPING
          };

        lock_type lcok (m_state_mutex);
        if (table[m_state][new_state] == 0)
          throw std::runtime_error ("invalid transition");
        else
          m_state = new_state;
      }

      void manager_t::initialize_memory_manager ()
      {
        gpi::api::gpi_api_t & gpi_api (gpi::api::gpi_api_t::get());
        global::memory_manager ().start ( gpi_api.rank ()
                                        , gpi_api.number_of_queues ()
                                        );

        if (global::topology ().is_master ())
        {
          global::memory_manager ().add_memory
            ( 0 // owner
            , m_default_memory_url
            , 1 // id
            );
        }
      }

      void manager_t::initialize_topology ()
      {
        gpi::api::gpi_api_t & gpi_api (gpi::api::gpi_api_t::get());
        global::topology().start( gpi_api.rank()
                                , global::topology_t::any_addr()
                                , global::topology_t::any_port() // topology_t::port_t("10821")
                                , "dummy-cookie"
                                );
        for (std::size_t n(0); n < gpi_api.number_of_nodes(); ++n)
        {
          if (gpi_api.rank() != n)
            global::topology().add_child(n);
        }
        global::topology().establish();
      }

      void manager_t::shutdown_topology ()
      {
        global::topology().stop();
      }

      void manager_t::attach_process (process_ptr_t proc)
      {
        lock_type lock (m_mutex);

        m_processes[proc->get_id()] = proc;
        m_processes[proc->get_id()]->start ();

        CLOG( INFO
            , "gpi.container"
            , "process container " << proc->get_id() << " attached"
            );
      }

      void manager_t::detach_process (const gpi::pc::type::process_id_t id)
      {
        lock_type lock (m_mutex);

        if (m_processes.find (id) == m_processes.end())
        {
          CLOG( ERROR
              , "gpi.container"
              , "process id already detached!"
              );
          throw std::runtime_error ("no such process");
        }

        process_ptr_t proc (m_processes.at (id));
        m_detached_processes.push_back (proc);
        m_processes.erase (id);
        proc->stop ();

        global::memory_manager().garbage_collect (id);

        CLOG( INFO
            , "gpi.container"
            , "process container " << id << " detached"
            );
      }

      void manager_t::garbage_collect ()
      {
        lock_type lock (m_mutex);
        while (!m_detached_processes.empty())
        {
          m_detached_processes.pop_front();
        }
      }

      void manager_t::handle_new_connection (int fd)
      {
        garbage_collect();

        gpi::pc::type::process_id_t proc_id (m_process_counter.inc());

        process_ptr_t proc (new process_type (*this, proc_id, fd));
        attach_process (proc);
      }

      void manager_t::handle_connector_error (int error)
      {
        if (error)
        {
          state_t st (get_state());
          if (st == ST_STOPPING || st == ST_STOPPED)
          {
            LOG( WARN
               , "ignoring error " << error << " (" << strerror(error) << ") in connector"
               );
          }
          else
          {
            LOG( ERROR
               , "connector had an error: " << strerror (error) << ", restarting it"
               );

            lock_type lock(m_mutex);
            m_connector.stop ();
            m_connector.start ();
          }
        }
      }

      void manager_t::handle_process_error( const gpi::pc::type::process_id_t proc_id
                                          , int error
                                          )
      {
        state_t st (get_state());
        if (st == ST_STOPPING || st == ST_STOPPED)
        {
          LOG_IF ( DEBUG
                 , error != 0
                 , "ignoring process container error during stop: pc = " << proc_id << " error = " << strerror(error)
                 );
        }
        else
        {
          LOG_IF ( ERROR
                 , error != 0
                 , "process container " << proc_id << " died: " << strerror (error)
                 );
          detach_process (proc_id);
        }
      }

      gpi::pc::type::segment_id_t manager_t::register_segment ( const gpi::pc::type::process_id_t pc_id
                                                              , std::string const & name
                                                              , const gpi::pc::type::size_t sz
                                                              , const gpi::pc::type::flags_t flags
                                                              )
      {
        // TODO: refactor here

        using namespace gpi::pc;

        fhg::util::url_t url;
        url.type ("shm");
        url.path (name);
        url.set ("size", boost::lexical_cast<std::string>(sz));
        if (flags & F_PERSISTENT)
          url.set ("persistent", "true");
        if (flags & F_EXCLUSIVE)
          url.set ("exclusive", "true");

        memory::area_ptr_t area =
          memory::factory ().create (boost::lexical_cast<std::string>(url));
        area->set_owner (pc_id);
        return global::memory_manager().register_memory (pc_id, area);
      }

      void manager_t::unregister_segment ( const gpi::pc::type::process_id_t proc_id
                                         , const gpi::pc::type::segment_id_t seg_id
                                         )
      {
        global::memory_manager().unregister_memory (proc_id, seg_id);
      }

      void manager_t::list_segments ( const gpi::pc::type::process_id_t proc_id
                                    , gpi::pc::type::segment::list_t &l
                                    ) const
      {
        global::memory_manager().list_memory (l);
      }

      void manager_t::attach_process_to_segment ( const gpi::pc::type::process_id_t proc_id
                                                , const gpi::pc::type::segment_id_t seg_id
                                                )
      {
        global::memory_manager().attach_process (proc_id, seg_id);
      }

      void manager_t::detach_process_from_segment ( const gpi::pc::type::process_id_t proc_id
                                                  , const gpi::pc::type::segment_id_t seg_id
                                                  )
      {
        global::memory_manager().detach_process (proc_id, seg_id);
      }

      void
      manager_t::collect_info (gpi::pc::type::info::descriptor_t &info) const
      {
        gpi::api::gpi_api_t & gpi_api (gpi::api::gpi_api_t::get());

        info.rank = gpi_api.rank();
        info.nodes = gpi_api.number_of_nodes();
        info.queues = gpi_api.number_of_queues();
        info.queue_depth = gpi_api.queue_depth();
      }

      gpi::pc::type::handle_t
      manager_t::alloc ( const gpi::pc::type::process_id_t proc_id
                       , const gpi::pc::type::segment_id_t seg_id
                       , const gpi::pc::type::size_t size
                       , const std::string & name
                       , const gpi::pc::type::flags_t flags
                       )
      {
//        check_permissions (permission::alloc_t (proc_id, seg_id));
        return global::memory_manager().alloc ( proc_id
                                              , seg_id
                                              , size
                                              , name
                                              , flags
                                              );
      }

      void
      manager_t::free ( const gpi::pc::type::process_id_t proc_id
                      , const gpi::pc::type::handle_id_t hdl
                      )
      {
//        check_permissions (permission::free_t (proc_id, hdl));
        global::memory_manager().free (hdl);
      }

      gpi::pc::type::handle::descriptor_t
      manager_t::info ( const gpi::pc::type::process_id_t
                      , const gpi::pc::type::handle_id_t hdl
                      ) const
      {
//        check_permissions (permission::info_t (proc_id, hdl));
        return global::memory_manager().info (hdl);
      }

      void
      manager_t::list_allocations ( const gpi::pc::type::process_id_t proc_id
                                  , const gpi::pc::type::segment_id_t seg_id
                                  , gpi::pc::type::handle::list_t & list
                                  ) const
      {
//        check_permissions (permission::list_allocations_t (proc_id, seg_id));
        if (seg_id == gpi::pc::type::segment::SEG_INVAL)
          global::memory_manager().list_allocations(list);
        else
          global::memory_manager().list_allocations (seg_id, list);
      }

      gpi::pc::type::queue_id_t
      manager_t::memcpy ( const gpi::pc::type::process_id_t proc_id
                        , gpi::pc::type::memory_location_t const & dst
                        , gpi::pc::type::memory_location_t const & src
                        , const gpi::pc::type::size_t amount
                        , const gpi::pc::type::queue_id_t queue
                        )
      {
        gpi::pc::type::validate (dst.handle);
        gpi::pc::type::validate (src.handle);
        return global::memory_manager().memcpy (proc_id, dst, src, amount, queue);
      }

      gpi::pc::type::size_t
      manager_t::wait_on_queue ( const gpi::pc::type::process_id_t proc_id
                               , const gpi::pc::type::queue_id_t queue
                               )
      {
        return global::memory_manager().wait_on_queue (proc_id, queue);
      }

      gpi::pc::type::segment_id_t
      manager_t::add_memory ( const gpi::pc::type::process_id_t proc_id
                            , std::string const &url
                            )
      {
        return global::memory_manager ().add_memory (proc_id, url);
      }

      void
      manager_t::del_memory ( const gpi::pc::type::process_id_t proc_id
                            , gpi::pc::type::segment_id_t seg_id
                            )
      {
        global::memory_manager ().del_memory (proc_id, seg_id);
      }
    }
  }
}
