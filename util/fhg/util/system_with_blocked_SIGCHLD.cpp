// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <fhg/syscall.hpp>

#include <boost/format.hpp>

#include <exception>
#include <cstdlib>

namespace fhg
{
  namespace util
  {
    int system_with_blocked_SIGCHLD (const char* command)
    {
      struct scoped_SIGCHLD_block
      {
        scoped_SIGCHLD_block()
        {
          sigset_t signals_to_block;
          fhg::syscall::sigemptyset (&signals_to_block);
          fhg::syscall::sigaddset (&signals_to_block, SIGCHLD);
          fhg::syscall::pthread_sigmask
            (SIG_BLOCK, &signals_to_block, &_signals_to_restore);
        }
        ~scoped_SIGCHLD_block()
        {
          fhg::syscall::pthread_sigmask
            (SIG_UNBLOCK, &_signals_to_restore, nullptr);
        }
        sigset_t _signals_to_restore;
      } const signal_blocker;

      return std::system (command);
    }

    void system_with_blocked_SIGCHLD_or_throw (std::string const& command)
    {
      if (int ec = fhg::util::system_with_blocked_SIGCHLD (command.c_str()))
      {
        throw std::runtime_error
          (( boost::format ("Could not run '%1%': error code '%2%'")
           % command
           % ec
           ).str()
          );
      }
    }
  }
}
