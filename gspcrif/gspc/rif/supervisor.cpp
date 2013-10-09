#include <boost/foreach.hpp>

#include <csignal>

#include <boost/bind.hpp>

#include <fhg/util/thread/async.hpp>

#include "supervisor.hpp"
#include "manager.hpp"

namespace gspc
{
  namespace rif
  {
    supervisor_t::supervisor_t ( manager_t & process_manager
                               , size_t max_restarts
                               , size_t max_start_time
                               , child_descriptor_list_t const & children
                               )
      : m_process_manager (process_manager)
      , m_max_restarts (max_restarts)
      , m_max_start_time (max_start_time)
    {
      m_process_manager.register_handler (this);

      BOOST_FOREACH (child_descriptor_t const &c, children)
      {
        if (0 != add_child (c))
        {
          throw std::runtime_error
            ("supervisor: could not start child: " + c.name);
        }
      }
    }

    supervisor_t::~supervisor_t ()
    {
      stop ();
    }

    void supervisor_t::start ()
    {
      onSupervisorStarted ();
    }

    void supervisor_t::stop ()
    {
      m_process_manager.unregister_handler (this);

      BOOST_FOREACH (child_t *c, m_children)
      {
        terminate_child (c->info.descriptor.name);
      }

      onSupervisorStopped ();
    }

    int supervisor_t::add_child (child_descriptor_t const &c)
    {
      {
        shared_lock lock (m_mutex);
        if (lookup_child (c.name) != 0)
          return -EEXIST;
      }

      child_t *child = new child_t;
      child->info.descriptor = c;
      child->info.proc = -1;
      child->info.started = 0;
      child->info.trial = 0;
      child->info.restart = c.restart_mode != child_descriptor_t::RESTART_NEVER;
      child->state = child_t::CHILD_TERMINATED;

      {
        unique_lock lock (m_mutex);
        m_children.push_back (child);
      }

      return start_child (c.name);
    }

    void supervisor_t::handle_terminated_child (child_t *child)
    {
      unique_lock lock (child->mutex);

      if (child->state == child_t::CHILD_TERMINATING)
      {
        child->state = child_t::CHILD_TERMINATED;

        lock.unlock ();

        onChildTerminated (child->info);

        return;
      }

      child->state = child_t::CHILD_TERMINATED;

      error_info_t error_info;
      m_process_manager.status (child->info.proc, &error_info.status);
      error_info.tstamp = time (NULL);

      char buffer [4096];
      ssize_t bytes_read;
      boost::system::error_code ec;

      bytes_read = m_process_manager.read (child->info.proc, 1, buffer, sizeof(buffer), ec);
      if (bytes_read > 0)
      {
        error_info.stdout.assign (buffer, bytes_read);
      }

      bytes_read = m_process_manager.read (child->info.proc, 2, buffer, sizeof(buffer), ec);
      if (bytes_read > 0)
      {
        error_info.stderr.assign (buffer, bytes_read);
      }
      child->info.errors.push_back (error_info);

      if (child->info.errors.size () > m_max_restarts)
        child->info.errors.pop_front ();

      m_process_manager.remove (child->info.proc);
      child->info.proc = 0;

      if (child->info.restart)
      {
        if (child->info.trial >= m_max_restarts)
        {
          lock.unlock ();

          onChildFailed (child->info);
        }
        else
        {
          lock.unlock ();

          onChildTerminated (child->info);

          if ((child->info.started + m_max_start_time) < (unsigned long)(time (NULL)))
          {
            // reset trial counter, child lived long enough
            child->info.trial = 0;
          }

          if (child->info.descriptor.restart_mode == child_descriptor_t::RESTART_ONLY_IF_FAILED)
          {
            if (error_info.status == 0)
            {
              return;
            }
          }

          if (child->info.restart)
          {
            fhg::thread::async (boost::bind ( &supervisor_t::start_child
                                            , this
                                            , child->info.descriptor.name
                                            )
                               );
          }
        }
      }
      else
      {
        lock.unlock ();

        if ( child->info.descriptor.restart_mode ==
           child_descriptor_t::RESTART_NEVER
           )
        {
          onChildTerminated (child->info);
        }
        else
        {
          onChildFailed (child->info);
        }
      }
    }

    void supervisor_t::onStateChange (proc_t p, process_state_t s)
    {
      child_t *child;

      {
        shared_lock lock (m_mutex);
        child = lookup_child (p);
        if (not child)
          return;
      }

      if (s == gspc::rif::PROCESS_TERMINATED)
      {
        fhg::thread::async (boost::bind ( &supervisor_t::handle_terminated_child
                                        , this
                                        , child
                                        )
                           );
      }
    }

    int supervisor_t::start_child (std::string const &name)
    {
      child_t *child = lookup_child (name);
      if (not child)
        return -ESRCH;

      {
        unique_lock lock (child->mutex);
        if (child->info.proc > 0)
          return 0;

        child->info.restart =
          child->info.descriptor.restart_mode != child_descriptor_t::RESTART_NEVER;

        child->info.proc = m_process_manager.exec ( child->info.descriptor.argv
                                                  , child->info.descriptor.env
                                                  );
        if (child->info.proc < 0)
        {
          child->state = child_t::CHILD_TERMINATED;
          return child->info.proc;
        }

        if (child->info.trial == 0)
        {
          child->info.started = time (NULL);
        }

        ++child->info.trial;
        child->state = child_t::CHILD_RUNNING;
      }

      onChildStarted (child->info);

      return 0;
    }

    int supervisor_t::remove_child (std::string const &name)
    {
      unique_lock lock (m_mutex);
      child_list_t::iterator child = m_children.begin ();
      const child_list_t::iterator end = m_children.end ();

      while (child != end)
      {
        if ((*child)->info.descriptor.name == name)
        {
          if ((*child)->info.proc > 0)
          {
            return -EINVAL;
          }
          else
          {
            delete (*child);
            m_children.erase (child);
            return 0;
          }
        }

        ++child;
      }

      return -ESRCH;
    }

    int supervisor_t::restart_child (std::string const &name)
    {
      int rc;

      rc = terminate_child (name);
      if (rc != 0)
        return rc;

      rc = start_child (name);

      return rc;
    }

    int supervisor_t::terminate_child (std::string const &name)
    {
      child_t *child = 0;

      {
        shared_lock lock (m_mutex);
        child = lookup_child (name);
        if (not child)
        {
          return -ESRCH;
        }
      }

      unique_lock clock (child->mutex);

      if (child->state == child_t::CHILD_TERMINATING)
        return 0;

      if (child->info.proc <= 0)
      {
        return 0;
      }

      child->state = child_t::CHILD_TERMINATING;

      child->info.restart = false;
      error_info_t error_info;
      error_info.status = -1;

      if (child->info.descriptor.shutdown_mode == child_descriptor_t::SHUTDOWN_INFINITY)
      {
        m_process_manager.kill (child->info.proc, SIGTERM);
        m_process_manager.wait (child->info.proc, &error_info.status);
      }
      else if (child->info.descriptor.shutdown_mode == child_descriptor_t::SHUTDOWN_WITH_TIMEOUT)
      {
        m_process_manager.kill (child->info.proc, SIGTERM);
        m_process_manager.wait ( child->info.proc
                               , &error_info.status
                               , boost::posix_time::milliseconds (child->info.descriptor.timeout)
                               );
      }

      // in any case: kill the process, if it should still be alive
      m_process_manager.kill (child->info.proc, SIGKILL);
      m_process_manager.wait (child->info.proc, &error_info.status);

      error_info.tstamp = time (NULL);

      char buffer [4096];
      size_t bytes_read;
      boost::system::error_code ec;

      bytes_read = m_process_manager.read (child->info.proc, 1, buffer, sizeof(buffer), ec);
      if (bytes_read > 0)
      {
        error_info.stdout.assign (buffer, bytes_read);
      }

      bytes_read = m_process_manager.read (child->info.proc, 2, buffer, sizeof(buffer), ec);
      if (bytes_read > 0)
      {
        error_info.stderr.assign (buffer, bytes_read);
      }
      child->info.errors.push_back (error_info);

      m_process_manager.remove (child->info.proc);
      child->info.proc = 0;

      if (child->info.errors.size () > m_max_restarts)
        child->info.errors.pop_front ();

      return 0;
    }

    const supervisor_t::child_t *supervisor_t::lookup_child (std::string const &name) const
    {
      BOOST_FOREACH (const child_t *c, m_children)
      {
        if (c->info.descriptor.name == name)
          return c;
      }
      return 0;
    }

    supervisor_t::child_t *supervisor_t::lookup_child (std::string const &name)
    {
      return const_cast<supervisor_t::child_t*>
        (static_cast<const supervisor_t&>(*this).lookup_child (name));
    }

    const supervisor_t::child_t *supervisor_t::lookup_child (const proc_t &pid) const
    {
      BOOST_FOREACH (const child_t *c, m_children)
      {
        if (c->info.proc == pid)
          return c;
      }
      return 0;
    }

    supervisor_t::child_t *supervisor_t::lookup_child (const proc_t &pid)
    {
      return const_cast<supervisor_t::child_t*>
        (static_cast<const supervisor_t&>(*this).lookup_child (pid));
    }

    std::list<std::string> supervisor_t::get_children () const
    {
      shared_lock lock (m_mutex);

      std::list<std::string> names;

      BOOST_FOREACH (const child_t *c, m_children)
      {
        names.push_back (c->info.descriptor.name);
      }

      return names;
    }

    supervisor_t::child_info_t const &supervisor_t::get_child_info (std::string const &name) const
    {
      const child_t *child = lookup_child (name);
      if (child)
      {
        return child->info;
      }
      else
      {
        throw std::runtime_error
          ("supervisor_t::get_child_info(" + name + "): no such child");
      }
    }
  }
}
