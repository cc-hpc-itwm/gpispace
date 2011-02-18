/* -*- mode: c++ -*- */

#include "manager.hpp"

#include <gpi-space/gpi/api.hpp>
#include <gpi-space/pc/segment/segment.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      manager_t::~manager_t ()
      {
        try
        {
          stop();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "error withing ~manager_t: " << ex.what());
        }
      }

      void manager_t::start ()
      {
        set_state (ST_STARTING);

        try
        {
          lock_type lock (m_mutex);
          m_connector.start ();

          initialize_segment_manager ();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "manager could not be started: connector: " << ex.what());
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
        }

        garbage_collect();

        set_state (ST_STOPPED);
      }

      manager_t::state_t manager_t::get_state () const
      {
        lock_type lcok (m_state_mutex);
        return m_state;
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

      void manager_t::initialize_segment_manager ()
      {
        gpi::api::gpi_api_t & gpi_api (gpi::api::gpi_api_t::get());
        m_segment_mgr.add_special_segment ( "global"
                                          , gpi::pc::type::segment::SEG_GLOBAL
                                          , gpi_api.memory_size()
                                          , gpi_api.dma_ptr()
                                          );
        m_segment_mgr.add_special_segment ( "local"
                                          , gpi::pc::type::segment::SEG_LOCAL
                                          , gpi_api.memory_size()
                                          , gpi_api.dma_ptr()
                                          );
      }

      gpi::pc::type::process_id_t manager_t::next_process_id ()
      {
        lock_type lock (m_mutex);
        gpi::pc::type::process_id_t proc_id (++m_process_id);
        if (0 == proc_id)
        {
          throw std::runtime_error
            ("could not generate new process id: overflow");
        }
        return proc_id;
      }

      void manager_t::attach_process (process_ptr_t proc)
      {
        lock_type lock (m_mutex);

        m_processes[proc->get_id()] = proc;
        m_processes[proc->get_id()]->start ();

        LOG( INFO
           , "process container " << proc->get_id() << " attached"
           );
      }

      void manager_t::detach_process (const gpi::pc::type::process_id_t id)
      {
        lock_type lock (m_mutex);

        if (m_processes.find (id) == m_processes.end())
        {
          LOG(WARN, "process id already detached!");
          return;
        }

        process_ptr_t proc (m_processes.at (id));
        m_processes.erase (id);
        proc->stop ();

        detach_segments_from_process (id);

        LOG( INFO
           , "process container " << id << " detached"
           );
        m_detached_processes.push_back (proc);
      }

      void manager_t::detach_segments_from_process (const gpi::pc::type::process_id_t proc_id)
      {
        lock_type lock (m_process_segment_relation_mutex);

        // TODO (ap):
        //       - cancel queued memory transfers
        //       - remove the segment
        //       - move it to the garbage area until transfers are complete
        segment_id_set_t & ids (m_process_segment_relation[proc_id]);
        while (! ids.empty())
        {
          detach_process_from_segment ( proc_id
                                      , *ids.begin()
                                      );
        }
        m_process_segment_relation.erase (proc_id);
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

        gpi::pc::type::process_id_t proc_id (next_process_id());

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
        gpi::pc::type::segment_id_t seg_id
          ( m_segment_mgr.register_segment (pc_id, name, sz, flags) );

        attach_process_to_segment (pc_id, seg_id);

        return seg_id;
      }

      void manager_t::unregister_segment ( const gpi::pc::type::process_id_t proc_id
                                         , const gpi::pc::type::segment_id_t seg_id
                                         )
      {
        detach_process_from_segment (proc_id, seg_id);
        m_segment_mgr.unregister_segment (seg_id);
      }

      bool manager_t::is_process_attached_to_segment ( const gpi::pc::type::process_id_t proc_id
                                                     , const gpi::pc::type::segment_id_t seg_id
                                                     ) const
      {
        lock_type lock (m_process_segment_relation_mutex);
        try
        {
          const segment_id_set_t & segments (m_process_segment_relation.at(proc_id));
          return segments.find (seg_id) != segments.end();
        }
        catch (std::exception const &ex)
        {
          return false;
        }
      }

      void manager_t::list_segments ( const gpi::pc::type::process_id_t proc_id
                                    , gpi::pc::type::segment::list_t &l
                                    ) const
      {
        m_segment_mgr.get_listing (l);

        for ( gpi::pc::type::segment::list_t::iterator s (l.begin())
            ; s != l.end()
            ; ++s
            )
        {
          if (is_process_attached_to_segment (proc_id, s->id))
          {
            gpi::flag::set (s->flags, gpi::pc::type::segment::F_ATTACHED);
          }
        }
      }

      void manager_t::attach_process_to_segment ( const gpi::pc::type::process_id_t proc_id
                                                , const gpi::pc::type::segment_id_t seg_id
                                                )
      {
        lock_type lock (m_process_segment_relation_mutex);

        // TODO:
        //    check permissions whether attaching is allowed
        //    if exclusive:
        //       creator == proc_id
        //
        if (m_process_segment_relation[proc_id].insert (seg_id).second)
        {
          m_segment_mgr.increment_refcount (seg_id);
        }
      }

      void manager_t::detach_process_from_segment ( const gpi::pc::type::process_id_t proc_id
                                                  , const gpi::pc::type::segment_id_t seg_id
                                                  )
      {
        lock_type lock (m_process_segment_relation_mutex);
        if (m_process_segment_relation.at (proc_id).erase (seg_id))
        {
          m_segment_mgr.decrement_refcount (seg_id);
        }
      }

      void manager_t::collect_info (gpi::pc::type::info::descriptor_t &info) const
      {
        gpi::api::gpi_api_t & gpi_api (gpi::api::gpi_api_t::get());

        info.rank = gpi_api.rank();
        info.nodes = gpi_api.number_of_nodes();
        info.queues = gpi_api.number_of_queues();
        info.queue_depth = gpi_api.queue_depth();
      }
    }
  }
}
