#ifndef GSPC_RIF_PROCESS_HPP
#define GSPC_RIF_PROCESS_HPP

#include <boost/utility.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <gspc/rif/types.hpp>
#include <gspc/rif/pipe.hpp>

namespace gspc
{
  namespace rif
  {
    class process_t : boost::noncopyable
    {
    public:
      explicit
      process_t ( proc_t id
                , boost::filesystem::path const &filename
                , argv_t const &
                , env_t const &
                );

      ~process_t ();

      int fork_and_exec ();

      pid_t pid () const;
      proc_t id () const;
      boost::optional<int> status () const;

      boost::filesystem::path const & filename () const;
      argv_t const & argv () const;
      env_t const & env () const;

      int wait ();
      int wait (boost::posix_time::time_duration td);
      void notify (int status);

      int kill (int sig);

      int waitpid ();
      int try_waitpid ();

      pipe_t & stdin ();
      pipe_t & stdout ();
      pipe_t & stderr ();

      ssize_t write (const void *buf, size_t len);
      ssize_t read (void *buf, size_t len);
      ssize_t readerr (void *buf, size_t len);
    private:
      int waitpid_and_notify (int flags);

      typedef boost::mutex              mutex_type;
      mutable mutex_type                m_mutex;
      mutable boost::condition_variable m_terminated;

      proc_t                  m_proc_id;
      boost::filesystem::path m_filename;
      argv_t                  m_argv;
      env_t                   m_env;

      pid_t  m_pid;
      boost::optional<int>    m_status;

      pipe_t m_stdin;
      pipe_t m_stdout;
      pipe_t m_stderr;
    };
  }
}

#endif
