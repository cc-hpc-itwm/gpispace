// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/syscall.hpp>

#include <boost/system/system_error.hpp>

#include <fcntl.h>
#include <signal.h>
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

    void bind (int sockfd, const struct sockaddr* addr, socklen_t addrlen)
    {
      return negative_one_fails_with_errno<void> (::bind (sockfd, addr, addrlen));
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

    void listen (int sockfd, int backlog)
    {
      return negative_one_fails_with_errno<void> (::listen (sockfd, backlog));
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

    void setsockopt (int sockfd, int level, int optname, const void* optval, socklen_t optlen)
    {
      return negative_one_fails_with_errno<void>
        (::setsockopt (sockfd, level, optname, optval, optlen));
    }

    void shutdown (int sockfd, int how)
    {
      return negative_one_fails_with_errno<void> (::shutdown (sockfd, how));
    }

    int socket (int domain, int type, int protocol)
    {
      return negative_one_fails_with_errno<int> (::socket (domain, type, protocol));
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
