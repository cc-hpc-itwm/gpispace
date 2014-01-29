// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/util/daemonize.hpp>

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

      const pid_t child (fork());
      if (child < 0)
      {
        throw std::runtime_error ("could not fork: " + std::string (strerror (errno)));
      }
      else if (child)
      {
        exit (EXIT_SUCCESS);
      }

      if (setsid() < 0)
      {
        throw std::runtime_error ("could not setsid: " + std::string (strerror (errno)));
      }
      if (chdir ("/") < 0)
      {
        throw std::runtime_error ("could not chdir: " + std::string (strerror (errno)));
      }

      umask (0);

      const pid_t child_child (fork());
      if (child_child < 0)
      {
        throw std::runtime_error ("could not fork (2): " + std::string (strerror (errno)));
      }
      else if (child_child)
      {
        return child_child;
      }

      if (close (0) < 0)
      {
        throw std::runtime_error ("could not close stdin: " + std::string (strerror (errno)));
      }
      if (close (1) < 0)
      {
        throw std::runtime_error ("could not close stdout: " + std::string (strerror (errno)));
      }
      if (close (2) < 0)
      {
        throw std::runtime_error ("could not close stderr: " + std::string (strerror (errno)));
      }

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
