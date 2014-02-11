// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/util/daemonize.hpp>

#include <fhg/syscall.hpp>

#include <stdexcept>

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace fhg
{
  namespace util
  {
    boost::optional<pid_t> fork_and_daemonize_child()
    {
      // this -> dies
      // -> child -> returns child_child
      //  -> child_child -> returns none

      if (fhg::syscall::fork())
      {
        exit (EXIT_SUCCESS);
      }

      fhg::syscall::setsid();
      if (chdir ("/") < 0)
      {
        throw std::runtime_error ("could not chdir: " + std::string (strerror (errno)));
      }

      umask (0);

      const pid_t child_child (fhg::syscall::fork());
      if (child_child)
      {
        return child_child;
      }

      fhg::syscall::close (0); // stdin
      fhg::syscall::close (1); // stdout
      fhg::syscall::close (2); // stderr

      if (open ("/dev/null", O_RDONLY) != 0)
      {
        throw std::runtime_error ("could not set stdin to /dev/null: " + std::string (strerror (errno)));
      }
      if (open ("/dev/null", O_WRONLY) != 1)
      {
        throw std::runtime_error ("could not set stdout to /dev/null: " + std::string (strerror (errno)));
      }
      if (open ("/dev/null", O_WRONLY) != 2)
      {
        throw std::runtime_error ("could not set stderr to /dev/null: " + std::string (strerror (errno)));
      }

      return boost::none;
    }

    void fork_and_daemonize_child_and_abandon_parent()
    {
      if (fork_and_daemonize_child())
      {
        exit (EXIT_SUCCESS);
      }
    }
  }
}
