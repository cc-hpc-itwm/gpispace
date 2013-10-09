#include "manager.hpp"
#include "util.hpp"

#include <errno.h>
#include <poll.h>               // poll()
#include <fcntl.h> // O_NONBLOCK
#include <signal.h>

#include <boost/foreach.hpp>
#include <boost/system/system_error.hpp>

#include <fhg/util/split.hpp>
#include <fhg/util/getenv.hpp>

#include "process.hpp"
#include "buffer.hpp"
#include "proc_info.hpp"
#include "process_handler.hpp"

namespace gspc
{
  namespace rif
  {
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

      namespace io_thread_command
      {
        enum io_thread_command_code
          {
            SHUTDOWN
          , NEWPROCESS
          };
      }

      struct io_thread_command_t
      {
        int command;
        int data;
      };
    }

    manager_t::manager_t ()
      : m_mutex ()
      , m_proc_ids ()
      , m_available_proc_ids ()
      , m_io_thread ()
      , m_io_thread_pipe ()
    {
      std::string val;
      if (0 == gspc::rif::getenv ("PATH", val))
        this->setenv ("PATH", val);
      if (0 == gspc::rif::getenv ("HOME", val))
        this->setenv ("HOME", val);
      if (0 == gspc::rif::getenv ("LD_LIBRARY_PATH", val))
        this->setenv ("LD_LIBRARY_PATH", val);
      if (0 == gspc::rif::getenv ("DISPLAY", val))
        this->setenv ("DISPLAY", val);
      if (0 == gspc::rif::getenv ("GSPC_COOKIE", val))
        this->setenv ("GSPC_COOKIE", val);
      if (0 == gspc::rif::getenv ("USER", val))
        this->setenv ("USER", val);
    }

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

      {
        while (not m_processes.empty ())
        {
          proc_t proc = m_processes.begin ()->first;
          this->term (proc, boost::posix_time::seconds (1));
          this->remove (proc);
        }
      }

      notify_io_thread (detail::io_thread_command::SHUTDOWN);

      m_io_thread->join ();
      m_io_thread.reset ();
      m_io_thread_pipe.close ();

      {
        unique_lock lock (m_mutex);
        m_stopping = false;
      }
    }

    void manager_t::setenv (std::string const &key, std::string const &val)
    {
      {
        unique_lock lock (m_mutex);
        m_environment [key] = val;
      }
      if (key == "PATH")
        update_search_path (val);
    }

    int manager_t::getenv (std::string const &key, std::string &val) const
    {
      shared_lock lock (m_mutex);
      env_t::const_iterator it = m_environment.find (key);
      if (it != m_environment.end ())
      {
        val = it->second;
        return 0;
      }
      else
      {
        return -1;
      }
    }

    void manager_t::delenv (std::string const &key)
    {
      {
        unique_lock lock (m_mutex);
        m_environment.erase (key);
      }
      if (key == "PATH")
        update_search_path ("");
    }

    env_t const &manager_t::env () const
    {
      shared_lock lock (m_mutex);
      return m_environment;
    }

    proc_t
    manager_t::exec (argv_t const & argv)
    {
      return this->exec (argv, m_environment);
    }

    proc_t
    manager_t::exec (argv_t const & argv, env_t const &env)
    {
      namespace fs = boost::filesystem;

      unique_lock lock (m_mutex);

      proc_t id;
      int rc;

      fs::path filename;
      rc = resolve (argv.front (), m_search_path, filename);
      if (rc < 0)
      {
        return rc;
      }

      if (not m_available_proc_ids.empty ())
      {
        id = m_available_proc_ids.top ();
        m_available_proc_ids.pop ();
      }
      else
      {
        id = ++m_proc_ids;
      }

      process_ptr_t p (new process_t (id, filename, argv, env, this));

      rc = p->fork_and_exec ();
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

      ::fcntl (p->stdin  ().wr (), F_SETFL, O_NONBLOCK);
      ::fcntl (p->stdout ().rd (), F_SETFL, O_NONBLOCK);
      ::fcntl (p->stderr ().rd (), F_SETFL, O_NONBLOCK);

      notify_io_thread (detail::io_thread_command::NEWPROCESS, id);

      return id;
    }

    proc_list_t manager_t::processes () const
    {
      proc_list_t pl;

      shared_lock lock (m_mutex);

      BOOST_FOREACH ( proc_map_t::value_type const &id_to_proc
                    , m_processes
                    )
      {
        pl.push_back (id_to_proc.first);
      }

      return pl;
    }

    void manager_t::onStateChange (proc_t p, process_state_t s)
    {
      shared_lock lock (m_process_handler_mutex);
      BOOST_FOREACH (process_handler_t *h, m_process_handler)
      {
        h->onStateChange (p, s);
      }
    }

    void manager_t::register_handler (process_handler_t *h)
    {
      unique_lock lock (m_process_handler_mutex);
      if ( std::find (m_process_handler.begin (), m_process_handler.end (), h)
         == m_process_handler.end ()
         )
        m_process_handler.push_back (h);
    }

    void manager_t::unregister_handler (process_handler_t *h)
    {
      unique_lock lock (m_process_handler_mutex);
      m_process_handler.remove (h);
    }

    int manager_t::proc_info (proc_t procid, proc_info_t &info) const
    {
      process_ptr_t p = process_by_id (procid);
      if (not p)
      {
        return -ESRCH;
      }

      info.assign_from (*p);

      return 0;
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

    int manager_t::status (proc_t proc, int *status)
    {
      process_ptr_t p = process_by_id (proc);
      if (not p)
        return -ESRCH;


      if (p->status ())
      {
        if (status)
          *status = *p->status ();
        return 0;
      }
      else
      {
        return -EBUSY;
      }
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

    ssize_t
    manager_t::read ( proc_t proc
                    , int fd
                    , char *buf, size_t len
                    , boost::system::error_code &ec
                    )
    {
      process_ptr_t p = process_by_id (proc);
      if (not p)
      {
        using namespace boost::system;
        ec = errc::make_error_code (errc::no_such_process);
        return -1;
      }

      return p->buffer (fd).read (buf, len);
    }

    ssize_t
    manager_t::write ( proc_t proc
                     , int fd
                     , const char *buf, size_t len
                     , boost::system::error_code &ec
                     )
    {
      process_ptr_t p = process_by_id (proc);
      if (not p)
      {
        using namespace boost::system;
        ec = errc::make_error_code (errc::no_such_process);
        return -1;
      }

      return p->buffer (fd).write (buf, len);
    }

    void manager_t::notify_io_thread (int cmd, int data) const
    {
      detail::io_thread_command_t s;
      s.command = cmd;
      s.data = data;

      m_io_thread_pipe.write (&s, sizeof(s));
    }

    manager_t::process_ptr_t manager_t::process_by_fd (int fd) const
    {
      fd_to_proc_map_t::const_iterator it = m_fd_to_proc.find (fd);
      if (it == m_fd_to_proc.end ())
        return process_ptr_t ();
      return process_by_id (it->second);
    }

    manager_t::process_ptr_t manager_t::process_by_pid (pid_t pid) const
    {
      pid_to_proc_map_t::const_iterator it = m_pid_to_proc.find (pid);
      if (it == m_pid_to_proc.end ())
        return process_ptr_t ();
      return process_by_id (it->second);
    }

    manager_t::process_ptr_t manager_t::process_by_id (proc_t proc) const
    {
      shared_lock lock (m_mutex);

      proc_map_t::const_iterator it = m_processes.find (proc);
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

    void manager_t::update_search_path (std::string const &val)
    {
      search_path_t sp;

      std::pair<std::string, std::string> head_tail;
      head_tail.second = val;

      while (not head_tail.second.empty ())
      {
        head_tail = fhg::util::split_string (head_tail.second, ":");
        if (not head_tail.first.empty ())
          sp.push_back (head_tail.first);
      }

      unique_lock lock (m_mutex);
      m_search_path = sp;
    }

    void manager_t::io_thread (pipe_t & me)
    {
      bool done = false;

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

            if (0 == p->try_waitpid ())
            {
              p->stdin ().close_wr ();
              p->stdout ().close_rd ();
              p->stderr ().close_rd ();

              continue;
            }

            // only add stdin if there is any data to be written
            if (p->stdin ().wr () >= 0 && p->buffer (STDIN_FILENO).size ())
              detail::add_pollfd (p->stdin ().wr (), POLLOUT, to_poll);
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
            detail::io_thread_command_t s;
            s.command = -1;

            ssize_t bytes = me.read (&s, sizeof(s));
            if (bytes != sizeof(s))
            {
              abort ();
            }

            switch (s.command)
            {
            case detail::io_thread_command::SHUTDOWN:
              done = true;
              break;
            case detail::io_thread_command::NEWPROCESS:
              break;
            default:
              break;
            }

            continue;
          }

          if (pfd.revents & POLLIN)
          {
            process_ptr_t p = process_by_fd (pfd.fd);

            if (p)
            {
              if (pfd.fd == p->stdout ().rd ())
              {
                p->buffer (STDOUT_FILENO).read_from (pfd.fd);
              }
              else if (pfd.fd == p->stderr ().rd ())
              {
                p->buffer (STDERR_FILENO).read_from (pfd.fd);
              }
              else
              {
                close (pfd.fd);
              }
            }
            else
            {
              close (pfd.fd);
            }
          }

          if (pfd.revents & POLLOUT)
          {
            process_ptr_t p = process_by_fd (pfd.fd);

            if (p)
            {
              if (pfd.fd == p->stdin ().wr ())
              {
                p->buffer (STDIN_FILENO).write_to (pfd.fd);
              }
              else
              {
                close (pfd.fd);
              }
            }
            else
            {
              close (pfd.fd);
            }
          }

          if (pfd.revents & POLLERR || pfd.revents & POLLNVAL)
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
          }
        }
      }
    }
  }
}
