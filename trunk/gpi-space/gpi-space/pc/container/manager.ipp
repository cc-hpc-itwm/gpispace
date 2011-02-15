/* -*- mode: c++ -*- */

#include "manager.hpp"

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
            m_processes.begin()->second->stop();
            m_processes.erase (m_processes.begin());
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

        // TODO
        //   - decrease segment reference
        //       - cancel queued memory transfers
        //       - remove the segment
        //          if (refcount == 0 && !segment.persistent)
        //          ongoing transfers are finished

        LOG( INFO
           , "process container " << id << " detached"
           );
        m_detached_processes.push_back (proc);
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
        lock_type lock (m_mutex);

        LOG(TRACE, "registering new shared memory segment: " << name << " with size " << sz << " via process " << pc_id);

        gpi::pc::segment::segment_t seg (name, sz);

        try
        {
          seg.open ();
          seg.assign_id (42);
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not register shared memory: " << ex.what());
          throw;
        }

        return seg.id ();
      }
    }
  }
}
