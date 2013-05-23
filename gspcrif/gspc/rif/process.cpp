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
        char **a_arg = (char**)(malloc (argv.size () + 1));
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
        char **a_env = (char**)(malloc (env.size () + 1));
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
      , m_status (0)
    {}

    process_t::~process_t ()
    {
      this->stdin ().close ();
      this->stdout ().close ();
      this->stderr ().close ();
    }

    int process_t::kill (int sig)
    {
      int rc;

      if (m_pid == -1)
      {
        return -ECHILD;
      }

      rc = ::kill (pid (), sig);
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

      // prepare pipes
      m_stdin.open  (0, false);
      m_stdout.open (0, false);
      m_stderr.open (0, false);

      pid_t new_pid = fork ();

      if (new_pid < 0)
      {
        int err = errno;
        m_stdin.close ();
        m_stdout.close ();
        m_stderr.close ();
        return -err;
      }

      if (0 == new_pid)
      {
        m_stdin.close_wr ();
        m_stdout.close_rd ();
        m_stderr.close_rd ();

        dup2 (m_stdin.rd (), STDIN_FILENO);
        dup2 (m_stdout.wr (), STDOUT_FILENO);
        dup2 (m_stderr.wr (), STDERR_FILENO);

        for (int fd = 3 ; fd < 1024 ; ++fd)
        {
          close (fd);
        }

        if (execve ( m_filename.string ().c_str ()
                   , detail::argv_to_array (m_argv)
                   , detail::env_to_array (m_env)
                   ) < 0
           )
        {
          int ec = 255;

          switch (errno)
          {
          case ENOENT:
            ec = 127;
            break;
          case ENOEXEC:
          case EPERM:
          case EACCES:
            ec = 126;
            break;
          case E2BIG:
          case EINVAL:
          case EIO:
          case EISDIR:
            ec = 125;
            break;
          default:
            ec = 255;
            break;
          }

          _exit (ec);
        }
      }
      else
      {
        m_pid = new_pid;
        m_stdin.close_rd ();
        m_stdout.close_wr ();
        m_stderr.close_wr ();
      }

      return 0;
    }

    pid_t process_t::pid () const
    {
      return m_pid;
    }

    proc_t process_t::proc_id () const
    {
      return m_proc_id;
    }

    int process_t::status () const
    {
      return m_status;
    }

    int process_t::wait ()
    {
      return this->wait (0);
    }

    int process_t::wait (int flags)
    {
      if (m_pid == -1)
      {
        return -ECHILD;
      }

      pid_t err = waitpid (m_pid, &m_status, flags);
      if (err < 0)
      {
        return -errno;
      }

      return err; // 0 -> not finished, >0 finished
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
      return m_stdin;
    }

    pipe_t & process_t::stdout ()
    {
      return m_stdout;
    }

    pipe_t & process_t::stderr ()
    {
      return m_stderr;
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
