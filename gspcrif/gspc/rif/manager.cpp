#include "manager.hpp"

#include <errno.h>
#include <poll.h>               // poll()
#include <fcntl.h> // O_NONBLOCK
#include <signal.h>

#include <boost/foreach.hpp>
#include <boost/system/system_error.hpp>

#include "process.hpp"

namespace gspc
{
  namespace rif
  {
    namespace io_thread_command
    {
      enum io_thread_cmd_e
        {
          SHUTDOWN
        , NEWPROCESS
        };
    }

    namespace detail
    {
      static void add_pollfd ( int fd
                             , int flags
                             , std::vector<pollfd> & to_poll
                             )
      {
        struct pollfd p;
        p.fd = fd;
        p.events = flags;
        p.revents = 0;
        to_poll.push_back (p);
      }
    }

    manager_t::manager_t ()
      : m_mutex ()
      , m_proc_ids ()
      , m_available_proc_ids ()
      , m_io_thread ()
      , m_io_thread_pipe ()
    {}

    manager_t::~manager_t ()
    {
      stop ();
    }

    void manager_t::start ()
    {
      int rc;

      unique_lock lock (m_mutex);
      if (m_io_thread)
        return;

      rc = m_io_thread_pipe.open (O_NONBLOCK, true);
      if (rc < 0)
      {
        using namespace boost::system;
        throw system_error (errc::make_error_code ((errc::errc_t)errno));
      }

      m_io_thread.reset
        (new boost::thread ( &manager_t::io_thread
                           , this
                           , boost::ref (m_io_thread_pipe)
                           ));
    }

    void manager_t::stop ()
    {
      {
        unique_lock lock (m_mutex);
        if (not m_io_thread)
        {
          return;
        }

        m_stopping = true;
      }

      // kill all procs

      notify_io_thread (io_thread_command::SHUTDOWN);

      m_io_thread->join ();
      m_io_thread.reset ();
      m_io_thread_pipe.close ();

      {
        unique_lock lock (m_mutex);
        m_stopping = false;
      }
    }

    proc_t
    manager_t::exec (argv_t const & argv)
    {
      return this->exec (argv, m_environment);
    }

    proc_t
    manager_t::exec (argv_t const & argv, env_t const &env)
    {
      unique_lock lock (m_mutex);

      proc_t id;

      if (not m_available_proc_ids.empty ())
      {
        id = m_available_proc_ids.top ();
        m_available_proc_ids.pop ();
      }
      else
      {
        id = ++m_proc_ids;
      }

      process_ptr_t p (new process_t (id, argv.front (), argv, env));

      int rc = p->fork_and_exec ();
      if (rc < 0)
      {
        m_available_proc_ids.push (p->id ());
        return rc;
      }

      m_processes [id] = p;
      m_fd_to_proc [p->stdin  ().wr ()] = p->id ();
      m_fd_to_proc [p->stdout ().rd ()] = p->id ();
      m_fd_to_proc [p->stderr ().rd ()] = p->id ();
      m_pid_to_proc [p->pid ()] = p->id ();

      ::fcntl (p->stdin ().wr (), F_SETFL, O_NONBLOCK);
      ::fcntl (p->stdout ().rd (), F_SETFL, O_NONBLOCK);
      ::fcntl (p->stderr ().rd (), F_SETFL, O_NONBLOCK);

      notify_io_thread (io_thread_command::NEWPROCESS);

      return id;
    }

    int manager_t::term (proc_t proc, boost::posix_time::time_duration td)
    {
      int rc;
      rc = this->kill (proc, SIGTERM);
      if (rc < 0)
        return rc;

      if (this->wait (proc, 0, td) != 0)
      {
        this->kill (proc, SIGKILL);
        this->wait (proc, 0, td);
      }
      return 0;
    }

    int manager_t::kill (proc_t proc, int sig)
    {
      process_ptr_t p = process_by_id (proc);
      if (not p)
        return -ESRCH;

      return p->kill (sig);
    }

    int manager_t::wait (proc_t proc, int *status)
    {
      return this->wait (proc, status, boost::posix_time::pos_infin);
    }

    int manager_t::wait ( proc_t proc
                        , int *status
                        , boost::posix_time::time_duration td
                        )
    {
      process_ptr_t p = process_by_id (proc);
      if (not p)
        return -ESRCH;

      int rc;
      rc = p->wait (td);
      if (0 == rc && status)
      {
        *status = *p->status ();
      }
      return rc;
    }

    void manager_t::notify_io_thread (int cmd) const
    {
      m_io_thread_pipe.write (&cmd, sizeof(cmd));
    }

    manager_t::process_ptr_t manager_t::process_by_fd (int fd)
    {
      fd_to_proc_map_t::iterator it = m_fd_to_proc.find (fd);
      if (it == m_fd_to_proc.end ())
        return process_ptr_t ();
      return process_by_id (it->second);
    }

    manager_t::process_ptr_t manager_t::process_by_pid (pid_t pid)
    {
      pid_to_proc_map_t::iterator it = m_pid_to_proc.find (pid);
      if (it == m_pid_to_proc.end ())
        return process_ptr_t ();
      return process_by_id (it->second);
    }

    manager_t::process_ptr_t manager_t::process_by_id (proc_t proc)
    {
      shared_lock lock (m_mutex);

      proc_map_t::iterator it = m_processes.find (proc);
      if (it == m_processes.end ())
        return process_ptr_t ();
      return it->second;
    }

    void
    manager_t::remove_fd_mapping (int fd)
    {
      unique_lock lock (m_mutex);
      m_fd_to_proc.erase (fd);
    }

    void
    manager_t::remove_pid_mapping (pid_t pid)
    {
      unique_lock lock (m_mutex);
      m_pid_to_proc.erase (pid);
    }

    int
    manager_t::remove (proc_t proc)
    {
      process_ptr_t p = process_by_id (proc);
      if (not p)
        return -ESRCH;

      if (not p->status ())
        return -EBUSY;

      remove_fd_mapping (p->stdin ().wr ());
      remove_fd_mapping (p->stdout ().rd ());
      remove_fd_mapping (p->stderr ().rd ());
      remove_pid_mapping (p->pid ());

      unique_lock lock (m_mutex);
      m_processes.erase (proc);
      m_available_proc_ids.push (proc);

      return 0;
    }

    void manager_t::io_thread (pipe_t & me)
    {
      bool done = false;

      char buf [4096];

      for (; not done ;)
      {
        int nready;

        // build poll array
        std::vector<pollfd> to_poll;
        detail::add_pollfd (me.rd (), POLLIN, to_poll);

        {
          shared_lock lock (m_mutex);
          BOOST_FOREACH ( proc_map_t::value_type const &id_to_proc
                        , m_processes
                        )
          {
            process_ptr_t p = id_to_proc.second;

            if (p->status ())
              continue;
            p->try_waitpid ();

            // only add stdin if there is any data to be written
            //detail::add_pollfd (p->stdin ().wr (), POLLOUT, to_poll);

            if (p->stdout ().rd () >= 0)
              detail::add_pollfd (p->stdout ().rd (), POLLIN, to_poll);
            if (p->stderr ().rd () >= 0)
              detail::add_pollfd (p->stderr ().rd (), POLLIN, to_poll);
          }
        }

        nready = poll (&to_poll [0], to_poll.size (), 500);

        if (nready < 0)
        {
          if (errno == EINTR)
            continue;
          using namespace boost::system;
          throw system_error (errc::make_error_code ((errc::errc_t)errno));
        }

        if (nready == 0)
        {
          continue;
        }

        BOOST_FOREACH (const pollfd &pfd, to_poll)
        {
          if (pfd.fd == me.rd () && pfd.revents & POLLIN)
          {
            int cmd = -1;
            ssize_t bytes = me.read (&cmd, sizeof(cmd));
            if (bytes != sizeof(cmd))
            {
              abort ();
            }

            switch (cmd)
            {
            case io_thread_command::SHUTDOWN:
              done = true;
              break;
            case io_thread_command::NEWPROCESS:
              break;
            default:
              break;
            }

            continue;
          }

          if (pfd.revents & POLLIN)
          {
            // lookup related process
            // read into process buffer
            //    if buffer full, do not poll for read

            ssize_t nbytes = read (pfd.fd, buf, sizeof(buf) - 1);
            if (nbytes > 0)
            {
              buf [nbytes] = 0;
              std::cerr << "read: '" << buf << "'" << std::endl;
            }
          }

          if (pfd.revents & POLLOUT)
          {
            std::cerr << "can write to " << pfd.fd << std::endl;
            // lookup related process
            // write pending data to process
            //    if buffer empty, do not poll for write anymore
          }

          if (pfd.revents & POLLERR || pfd.revents & POLLNVAL)
          {
            std::cerr << "error on " << pfd.fd << std::endl;
            // lookup related process and see what can be done about it
            //    close the other side of the pipe
          }

          if (pfd.revents & POLLHUP)
          {
            process_ptr_t p = process_by_fd (pfd.fd);
            if (p)
            {
              if (pfd.fd == p->stdin ().wr ())
              {
                p->stdin ().close_wr ();
              }
              else if (pfd.fd == p->stdout ().rd ())
              {
                p->stdout ().close_rd ();
              }
              else if (pfd.fd == p->stderr ().rd ())
              {
                p->stderr ().close_rd ();
              }
              remove_fd_mapping (pfd.fd);
            }
            else
            {
              abort ();
            }
          }
        }
      }
    }
  }
}
