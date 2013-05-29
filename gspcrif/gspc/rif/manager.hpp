#ifndef GSPC_RIF_MANAGER_HPP
#define GSPC_RIF_MANAGER_HPP

#include <stack>

#include <boost/utility.hpp>      // noncopyable
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp> // time_duration
#include <boost/unordered_map.hpp>

#include <fhg/util/thread/atomic.hpp>

#include <gspc/rif/types.hpp>
#include <gspc/rif/pipe.hpp>

namespace gspc
{
  namespace rif
  {
    class proc_info_t;
    class proc_handler_t;
    class process_t;

    class manager_t : boost::noncopyable
    {
    public:
      manager_t ();
      ~manager_t ();

      void start ();
      void stop ();

      void setenv (std::string const &key, std::string const &val);
      int  getenv (std::string const &key, std::string &val) const;
      void delenv (std::string const &key);
      env_t const &env () const;

      /**
         executes the given command

         @return -ERRNO if something went wrong, the process id otherwise
       */
      proc_t exec (argv_t const &);

      /**
         executes the given command with the given environment

         @return -ERRNO if something went wrong, the process id otherwise
       */
      proc_t exec (argv_t const &, env_t const &);

      /**
         convenience function to terminate a given process
       */
      int term (proc_t proc, boost::posix_time::time_duration t);

      /**
         send the given signal to the specified process
       */
      int kill (proc_t proc, int sig);

      /**
         query the status of the given process
       */
      int status (proc_t proc, int *status);

      /**
         Removes the internal information about the process.

         This call fails when the process is still running.

         The id behind 'proc' will be available for subsequent exec calls.
       */
      int remove (proc_t proc);

      int wait (proc_t proc, int *status);
      int wait (proc_t proc, int *status, boost::posix_time::time_duration t);

      ssize_t read (proc_t, int fd, char *, size_t len, boost::system::error_code &);
      ssize_t write (proc_t, int fd, const char *, size_t len, boost::system::error_code &);

      /**
         Retrieve additional information about the process.

         @return 0 on success, -ERRNO otherwhise
       */
      int proc_info (proc_t, proc_info_t &) const;

      /**
         Retrieve the list of processes.
       */
      proc_list_t processes () const;

      /**
         Register a  callback handler that  will be  run when a  process changes
         state.
       */
      void register_handler (proc_handler_t);
    private:
      typedef boost::shared_lock<boost::shared_mutex> shared_lock;
      typedef boost::unique_lock<boost::shared_mutex> unique_lock;

      typedef boost::shared_ptr<process_t> process_ptr_t;
      typedef boost::unordered_map<proc_t, process_ptr_t> proc_map_t;
      typedef boost::unordered_map<pid_t, proc_t> pid_to_proc_map_t;
      typedef boost::unordered_map<int, proc_t> fd_to_proc_map_t;

      void io_thread (pipe_t &);
      void notify_io_thread (int cmd) const;

      process_ptr_t process_by_fd (int) const;
      void remove_fd_mapping (int);

      process_ptr_t process_by_pid (pid_t) const;
      void remove_pid_mapping (pid_t);

      process_ptr_t process_by_id (proc_t) const;

      void update_search_path (std::string const &val);

      bool m_stopping;

      mutable boost::shared_mutex m_mutex;
      fhg::thread::atomic<proc_t> m_proc_ids;
      std::stack<proc_t>          m_available_proc_ids;
      search_path_t               m_search_path;

      boost::shared_ptr<boost::thread> m_io_thread;
      mutable pipe_t                   m_io_thread_pipe;

      env_t             m_environment;
      proc_map_t        m_processes;
      pid_to_proc_map_t m_pid_to_proc;
      fd_to_proc_map_t  m_fd_to_proc;
    };
  }
}

#endif
