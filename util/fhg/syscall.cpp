// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/syscall.hpp>

#include <boost/system/system_error.hpp>

#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>

namespace fhg
{
  namespace syscall
  {
    namespace
    {
      template<typename T>
        T negative_one_fails_with_errno (int rc)
      {
        if (rc == -1)
        {
          throw boost::system::system_error
            (boost::system::error_code (errno, boost::system::system_category()));
        }
        return T (rc);
      }

      template<>
        void negative_one_fails_with_errno<void> (int rc)
      {
        if (rc == -1)
        {
          throw boost::system::system_error
            (boost::system::error_code (errno, boost::system::system_category()));
        }
      }
    }

    void chdir (const char* path)
    {
      return negative_one_fails_with_errno<void> (::chdir (path));
    }

    void chmod (const char* path, mode_t mode)
    {
      return negative_one_fails_with_errno<void> (::chmod (path, mode));
    }

    void close (int fd)
    {
      return negative_one_fails_with_errno<void> (::close (fd));
    }

    int dup (int oldfd)
    {
      return negative_one_fails_with_errno<int> (::dup (oldfd));
    }
    int dup (int oldfd, int newfd)
    {
      return negative_one_fails_with_errno<int> (::dup2 (oldfd, newfd));
    }
    int dup (int oldfd, int newfd, int flags)
    {
      return negative_one_fails_with_errno<int> (::dup3 (oldfd, newfd, flags));
    }

    void execve (const char* filename, char* const argv[], char* const envp[])
    {
      return negative_one_fails_with_errno<void> (::execve (filename, argv, envp));
    }

    pid_t fork()
    {
      return negative_one_fails_with_errno<pid_t> (::fork());
    }

    void kill (pid_t pid, int sig)
    {
      return negative_one_fails_with_errno<void> (::kill (pid, sig));
    }

    int open (const char* pathname, int flags)
    {
      return negative_one_fails_with_errno<int> (::open (pathname, flags));
    }
    int open (const char* pathname, int flags, mode_t mode)
    {
      return negative_one_fails_with_errno<int> (::open (pathname, flags, mode));
    }

    pid_t setsid()
    {
      return negative_one_fails_with_errno<pid_t> (::setsid());
    }

    void shutdown (int sockfd, int how)
    {
      return negative_one_fails_with_errno<void> (::shutdown (sockfd, how));
    }

    void unlink (const char* pathname)
    {
      return negative_one_fails_with_errno<void> (::unlink (pathname));
    }

    pid_t wait (pid_t pid, int* status, int options, struct rusage* rusage)
    {
      return negative_one_fails_with_errno<pid_t>
        (::wait4 (pid, status, options, rusage));
    }
  }
}
