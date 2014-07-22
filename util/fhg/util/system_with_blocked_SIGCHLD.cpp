// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <fhg/syscall.hpp>

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
  }
}
