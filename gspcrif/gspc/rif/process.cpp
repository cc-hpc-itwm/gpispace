#include "process.hpp"

#include <errno.h>
#include <unistd.h>             // char **environ
#include <stdlib.h>             // malloc
#include <stdio.h>              // snprintf
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>           // waitpid
#include <signal.h>             // kill

#include <stdexcept>
#include <boost/format.hpp>

#include <fhg/util/split.hpp>

#include "util.hpp"
#include "buffer.hpp"
#include "null_process_handler.hpp"

namespace fs = boost::filesystem;

namespace gspc
{
  namespace rif
  {
    namespace detail
    {
      static char **argv_to_array (argv_t const &argv)
      {
        char **a_arg = reinterpret_cast<char**>(malloc ((argv.size () + 1) * sizeof(char*)));
        a_arg [argv.size ()] = (char*)0;
        for (size_t i = 0 ; i < argv.size () ; ++i)
        {
          a_arg [i] = reinterpret_cast<char*>(malloc (argv[i].size ()+1));
          memcpy (a_arg [i], argv[i].c_str (), argv [i].size ()+1);
        }

        return a_arg;
      }

      static char **env_to_array (env_t const &env)
      {
        char **a_env = reinterpret_cast<char**>(malloc ((env.size () + 1) * sizeof(char*)));
        a_env [env.size ()] = (char*)0;

        env_t::const_iterator it = env.begin ();
        const env_t::const_iterator end = env.end ();

        size_t i = 0;
        while (it != end)
        {
          const size_t length_of_entry =
            //         key      =             value     \0
            it->first.size () + 1 + it->second.size () + 1;
          a_env [i] = reinterpret_cast<char*>(malloc (length_of_entry));

          snprintf ( a_env [i]
                   , length_of_entry
                   , "%s=%s", it->first.c_str (), it->second.c_str ()
                   );

          ++i;
          ++it;
        }

        return a_env;
      }
    }

    process_t::process_t ( proc_t id
                         , boost::filesystem::path const &filename
                         , argv_t const &argv
                         , env_t const &env
                         , process_handler_t *handler
                         )
      : m_proc_id (id)
      , m_state (PROCESS_CREATED)
      , m_filename (fs::absolute (filename))
      , m_argv (argv)
      , m_env (env)
      , m_pid (-1)
      , m_status ()
      , m_handler (handler)
    {
      for (size_t i = 0 ; i < 3 ; ++i)
      {
        m_pipes.push_back (pipe_t ());
        m_buffers.push_back (new buffer_t (2097152));
      }

      set_state (PROCESS_CREATED);
    }

    process_t::process_t ( proc_t id
                         , boost::filesystem::path const &filename
                         , argv_t const &argv
                         , process_handler_t *handler
                         )
      : m_proc_id (id)
      , m_state (PROCESS_CREATED)
      , m_filename (fs::absolute (filename))
      , m_argv (argv)
      , m_env ()
      , m_pid (-1)
      , m_status ()
      , m_handler (handler)
    {
      for (size_t i = 0 ; i < 3 ; ++i)
      {
        m_pipes.push_back (pipe_t ());
        m_buffers.push_back (new buffer_t (2097152));
      }

      // initialize environment from my own
      char **env_entry = environ;
      while (*env_entry)
      {
        std::pair<std::string, std::string> kv =
          fhg::util::split (*env_entry, "=");

        m_env [kv.first] = kv.second;

        ++env_entry;
      }

      set_state (PROCESS_CREATED);
    }

    process_t::~process_t ()
    {
      for (size_t i = 0 ; i < m_pipes.size () ; ++i)
      {
        m_pipes [i].close ();
      }
      for (size_t i = 0 ; i < m_buffers.size () ; ++i)
      {
        delete m_buffers [i];
      }
    }

    int process_t::kill (int sig)
    {
      int rc;

      if (m_status)
        return 0;

      if (m_pid == -1)
      {
        return -ECHILD;
      }

      rc = ::kill (m_pid, sig);
      if (rc < 0)
        return -errno;
      else
        return 0;
    }

    void process_t::set_state (process_state_t s)
    {
      if (m_state != s)
      {
        m_state = s;
        m_handler->onStateChange (m_proc_id, m_state);
      }
    }

    int process_t::fork_and_exec ()
    {
      if (m_pid != -1)
        return -EINVAL;

      if (m_argv.empty ())
        return -EINVAL;

      for (size_t i = 0 ; i < m_pipes.size () ; ++i)
      {
        m_pipes [i].open (0, false);
      }

      pid_t new_pid = fork ();

      if (new_pid < 0)
      {
        int err = errno;
        for (size_t i = 0 ; i < m_pipes.size () ; ++i)
          m_pipes [i].close ();

        set_state (PROCESS_FAILED);
        return -err;
      }

      if (0 == new_pid)
      {
        this->stdin ().close_wr ();
        this->stdout ().close_rd ();
        this->stderr ().close_rd ();

        dup2 (this->stdin ().rd () , STDIN_FILENO);
        dup2 (this->stdout ().wr (), STDOUT_FILENO);
        dup2 (this->stderr ().wr (), STDERR_FILENO);

        for (int fd = 3 ; fd < 1024 ; ++fd)
        {
          ::close (fd);
        }

        chdir ("/");

        if (execve ( m_filename.string ().c_str ()
                   , detail::argv_to_array (m_argv)
                   , detail::env_to_array (m_env)
                   ) < 0
           )
        {
          int ec;

          switch (errno)
          {
          case ENOENT:
            ec = 127;
            break;
          case ENOEXEC:
          case EPERM:
          case EFAULT:
          case EACCES:
          case E2BIG:
          case EINVAL:
          case EIO:
          case EISDIR:
          default:
            ec = 126;
            break;
          }

          _exit (ec);
        }
      }
      else
      {
        m_pid = new_pid;
        this->stdin ().close_rd ();
        this->stdout ().close_wr ();
        this->stderr ().close_wr ();

        set_state (PROCESS_STARTED);
      }

      return 0;
    }

    pid_t process_t::pid () const
    {
      return m_pid;
    }

    const struct rusage *process_t::rusage () const
    {
      return &m_rusage;
    }

    proc_t process_t::id () const
    {
      shared_lock lock (m_mutex);
      return m_proc_id;
    }

    gspc::rif::process_state_t process_t::state () const
    {
      shared_lock lock (m_mutex);
      return m_state;
    }

    boost::optional<int> process_t::status () const
    {
      shared_lock lock (m_mutex);
      return m_status;
    }

    int process_t::exit_code () const
    {
      shared_lock lock (m_mutex);

      if (m_status)
      {
        return make_exit_code (*m_status);
      }

      return -EBUSY;
    }

    int process_t::wait ()
    {
      return this->wait (boost::posix_time::pos_infin);
    }

    int process_t::wait (boost::posix_time::time_duration td)
    {
      unique_lock lock (m_mutex);

      while (not m_status)
      {
        m_terminated.timed_wait (lock, td);
        if (not m_status)
          return -ETIME;
      }

      return 0;
    }

    int process_t::try_waitpid ()
    {
      return waitpid_and_notify (WNOHANG);
    }

    int process_t::waitpid ()
    {
      return waitpid_and_notify (0);
    }

    void process_t::notify (int s)
    {
      boost::unique_lock<mutex_type> lock (m_mutex);

      if (not m_status)
      {
        m_status = s;
        m_pid = -1;
      }

      lock.unlock ();

      set_state (PROCESS_TERMINATED);
      m_terminated.notify_all ();
    }

    int process_t::waitpid_and_notify (int flags)
    {
      int s;
      pid_t err = ::wait4 (m_pid, &s, flags, &m_rusage);
      if (err < 0)
      {
        return -errno;
      }

      if (err == m_pid)
      {
        this->notify (s);
        return 0;
      }
      else
      {
        return -EBUSY;
      }
    }

    boost::filesystem::path const & process_t::filename () const
    {
      return m_filename;
    }

    argv_t const & process_t::argv () const
    {
      return m_argv;
    }

    env_t const & process_t::env () const
    {
      return m_env;
    }

    pipe_t & process_t::stdin ()
    {
      return m_pipes [STDIN_FILENO];
    }

    pipe_t & process_t::stdout ()
    {
      return m_pipes [STDOUT_FILENO];
    }

    pipe_t & process_t::stderr ()
    {
      return m_pipes [STDERR_FILENO];
    }

    ssize_t process_t::read (void *buffer, size_t len)
    {
      return stdout ().read (buffer, len);
    }

    ssize_t process_t::readerr (void *buffer, size_t len)
    {
      return stderr ().read (buffer, len);
    }

    ssize_t process_t::write (const void *buffer, size_t len)
    {
      return stdin ().write (buffer, len);
    }

    buffer_t & process_t::buffer (int fd)
    {
      if (fd < 0 || (size_t)fd >= m_buffers.size ())
        throw std::out_of_range
          ((boost::format ("process_t::buffer(): fd out of range: %1%") % fd).str ());
      return *m_buffers [fd];
    }

    buffer_t const & process_t::buffer (int fd) const
    {
      if (fd < 0 || (size_t)fd >= m_buffers.size ())
        throw std::out_of_range
          ((boost::format ("process_t::buffer(): fd out of range: %1%") % fd).str ());
      return *m_buffers [fd];
    }
  }
}
