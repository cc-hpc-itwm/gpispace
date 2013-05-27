#include "process.hpp"

#include <errno.h>
#include <stdlib.h>             // malloc
#include <stdio.h>              // snprintf
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>           // waitpid
#include <signal.h>             // kill

namespace fs = boost::filesystem;

namespace gspc
{
  namespace rif
  {
    namespace detail
    {
      static char **argv_to_array (argv_t const &argv)
      {
        char **a_arg = (char**)(malloc ((argv.size () + 1) * sizeof(char*)));
        a_arg [argv.size ()] = (char*)0;
        for (size_t i = 0 ; i < argv.size () ; ++i)
        {
          a_arg [i] = (char*)(malloc (argv[i].size ()+1));
          memcpy (a_arg [i], argv[i].c_str (), argv [i].size ()+1);
        }

        return a_arg;
      }

      static char **env_to_array (env_t const &env)
      {
        char **a_env = (char**)(malloc ((env.size () + 1) * sizeof(char*)));
        a_env [env.size ()] = (char*)0;

        env_t::const_iterator it = env.begin ();
        const env_t::const_iterator end = env.end ();

        size_t i = 0;
        while (it != end)
        {
          const size_t length_of_entry =
            //         key      =             value     \0
            it->first.size () + 1 + it->second.size () + 1;
          a_env [i] = (char*)(malloc (length_of_entry));

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
                         )
      : m_proc_id (id)
      , m_filename (fs::absolute (filename))
      , m_argv (argv)
      , m_env (env)
      , m_pid (-1)
      , m_status ()
    {
      for (size_t i = 0 ; i < 3 ; ++i)
      {
        m_pipes.push_back (pipe_t ());
      }
    }

    process_t::~process_t ()
    {
      for (size_t i = 0 ; i < m_pipes.size () ; ++i)
        m_pipes [i].close ();
    }

    int process_t::kill (int sig)
    {
      int rc;

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
      }

      return 0;
    }

    pid_t process_t::pid () const
    {
      return m_pid;
    }

    proc_t process_t::id () const
    {
      shared_lock lock (m_mutex);
      return m_proc_id;
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
        if (WIFEXITED (*m_status))
        {
          return WEXITSTATUS (*m_status) & 0xff;
        }
        else if (WIFSIGNALED (*m_status))
        {
          return (128 + WTERMSIG (*m_status)) & 0xff;
        }
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
      }
      m_terminated.notify_all ();
    }

    int process_t::waitpid_and_notify (int flags)
    {
      int s;
      pid_t err = ::waitpid (m_pid, &s, flags);
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
  }
}
