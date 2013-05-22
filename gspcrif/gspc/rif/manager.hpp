#ifndef GSPC_RIF_MANAGER_HPP
#define GSPC_RIF_MANAGER_HPP

#include <gspc/rif/types.hpp>

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace gspc
{
  namespace rif
  {
    class proc_info_t;
    class proc_handler_t;

    class manager_t
    {
    public:
      manager_t ();
      ~manager_t ();

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
      int signal (proc_t proc, int sig);

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
    };
  }
}

#endif
