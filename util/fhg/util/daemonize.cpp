// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/util/daemonize.hpp>

#include <fhg/syscall.hpp>

#include <boost/asio/io_service.hpp>

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
      fhg::syscall::chdir ("/");

      umask (0);

      const pid_t child_child (fhg::syscall::fork());
      if (child_child)
      {
        return child_child;
      }

      fhg::syscall::close (0); // stdin
      fhg::syscall::close (1); // stdout
      fhg::syscall::close (2); // stderr

      if (fhg::syscall::open ("/dev/null", O_RDONLY) != 0)
      {
        throw std::runtime_error ("could not set stdin to /dev/null: open did not return fd 0");
      }
      if (fhg::syscall::open ("/dev/null", O_WRONLY) != 1)
      {
        throw std::runtime_error ("could not set stdout to /dev/null: open did not return fd 1");
      }
      if (fhg::syscall::open ("/dev/null", O_WRONLY) != 2)
      {
        throw std::runtime_error ("could not set stderr to /dev/null: open did not return fd 2");
      }

      return boost::none;
    }

    void fork_and_daemonize_child_and_abandon_parent()
    {
      fork_and_daemonize_child_and_abandon_parent ({});
    }

    void fork_and_daemonize_child_and_abandon_parent
      (std::initializer_list<boost::asio::io_service*> io_services)
    {
      for (boost::asio::io_service* io_service : io_services)
      {
        io_service->notify_fork (boost::asio::io_service::fork_prepare);
      }
      if (fork_and_daemonize_child())
      {
        for (boost::asio::io_service* io_service : io_services)
        {
          io_service->notify_fork (boost::asio::io_service::fork_parent);
        }
        exit (EXIT_SUCCESS);
      }
      for (boost::asio::io_service* io_service : io_services)
      {
        io_service->notify_fork (boost::asio::io_service::fork_child);
      }
    }
  }
}
