// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/syscall.hpp>

#include <boost/system/system_error.hpp>

#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

namespace fhg
{
  namespace syscall
  {
    namespace
    {
      template<typename T, typename R>
        T negative_one_fails_with_errno (R rc)
      {
        if (rc == R (-1))
        {
          throw boost::system::system_error
            (boost::system::error_code (errno, boost::system::system_category()));
        }
        return rc;
      }

      template<typename T>
      T *nullptr_fails_with_errno (T *rc)
      {
        if (rc == nullptr)
        {
          throw boost::system::system_error
            (boost::system::error_code (errno, boost::system::system_category()));
        }
        return rc;
      }

      template<>
        void negative_one_fails_with_errno<void, int> (int rc)
      {
        if (rc == -1)
        {
          throw boost::system::system_error
            (boost::system::error_code (errno, boost::system::system_category()));
        }
      }

      void* MAP_FAILED_fails_with_errno (void* rc)
      {
        if (rc == MAP_FAILED)
        {
          throw boost::system::system_error
            (boost::system::error_code (errno, boost::system::system_category()));
        }
        return rc;
      }

      sighandler_t SIG_ERR_fails_with_errno (sighandler_t rc)
      {
        if (rc == SIG_ERR)
        {
          throw boost::system::system_error
            (boost::system::error_code (errno, boost::system::system_category()));
        }
        return rc;
      }

      template<typename R>
        void non_zero_is_error_code (R rc)
      {
        if (rc != R (0))
        {
          throw boost::system::system_error
            (boost::system::error_code (rc, boost::system::system_category()));
        }
      }

      template<typename R>
        R cannot_fail (R rc)
      {
        return rc;
      }
    }

    int accept (int sockfd, struct sockaddr* addr, socklen_t* addrlen)
    {
      return negative_one_fails_with_errno<int> (::accept (sockfd, addr, addrlen));
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

    void execve (const char* filename, char* const argv[], char* const envp[])
    {
      negative_one_fails_with_errno<void> (::execve (filename, argv, envp));

      abort(); // execve either does not return, or returns negative, thus throws
    }

    pid_t fork()
    {
      return negative_one_fails_with_errno<pid_t> (::fork());
    }

    void ftruncate (int fd, off_t length)
    {
      return negative_one_fails_with_errno<void> (::ftruncate (fd, length));
    }

    char* getenv (const char* name)
    {
      return cannot_fail (::getenv (name));
    }

    pid_t getpid()
    {
      return cannot_fail (::getpid());
    }

    uid_t getuid()
    {
      return cannot_fail (::getuid());
    }

    void kill (pid_t pid, int sig)
    {
      return negative_one_fails_with_errno<void> (::kill (pid, sig));
    }

    void listen (int sockfd, int backlog)
    {
      return negative_one_fails_with_errno<void> (::listen (sockfd, backlog));
    }

    void* mmap (void* addr, size_t length, int prot, int flags, int fd, off_t offset)
    {
      return MAP_FAILED_fails_with_errno
        (::mmap (addr, length, prot, flags, fd, offset));
    }

    void mkfifo (const char* pathname, mode_t mode)
    {
      return negative_one_fails_with_errno<void> (::mkfifo (pathname, mode));
    }

    void munmap (void* addr, size_t length)
    {
      return negative_one_fails_with_errno<void> (::munmap (addr, length));
    }

    int open (const char* pathname, int flags)
    {
      return negative_one_fails_with_errno<int> (::open (pathname, flags));
    }
    int open (const char* pathname, int flags, mode_t mode)
    {
      return negative_one_fails_with_errno<int> (::open (pathname, flags, mode));
    }

    void pipe (int pipefd[2])
    {
      return negative_one_fails_with_errno<void> (::pipe (pipefd));
    }

    void pthread_sigmask (int how, const sigset_t* set, sigset_t* oset)
    {
      return non_zero_is_error_code (::pthread_sigmask (how, set, oset));
    }

    ssize_t read (int fd, void* buf, size_t count)
    {
      return negative_one_fails_with_errno<ssize_t> (::read (fd, buf, count));
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

    int shm_open (const char* name, int oflag, mode_t mode)
    {
      return negative_one_fails_with_errno<int> (::shm_open (name, oflag, mode));
    }

    void shm_unlink (const char* name)
    {
      return negative_one_fails_with_errno<void> (::shm_unlink (name));
    }

    void shutdown (int sockfd, int how)
    {
      return negative_one_fails_with_errno<void> (::shutdown (sockfd, how));
    }

    void sigaction (int signum, const struct sigaction* act, struct sigaction* oldact)
    {
      return negative_one_fails_with_errno<void>
        (::sigaction (signum, act, oldact));
    }

    void sigaddset (sigset_t* set, int signum)
    {
      return negative_one_fails_with_errno<void> (::sigaddset (set, signum));
    }

    void sigemptyset (sigset_t* set)
    {
      return negative_one_fails_with_errno<void> (::sigemptyset (set));
    }

    sighandler_t signal (int signum, sighandler_t handler)
    {
      return SIG_ERR_fails_with_errno (::signal (signum, handler));
    }

    int socket (int domain, int type, int protocol)
    {
      return negative_one_fails_with_errno<int> (::socket (domain, type, protocol));
    }

    void stat (const char* path, struct stat* buf)
    {
      return negative_one_fails_with_errno<void> (::stat (path, buf));
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

    pid_t waitpid (pid_t pid, int* status, int options)
    {
      return negative_one_fails_with_errno<pid_t>
        (::waitpid (pid, status, options));
    }

    ssize_t write (int fd, const void* buf, size_t count)
    {
      return negative_one_fails_with_errno<ssize_t> (::write (fd, buf, count));
    }

    int connect (int sock, const struct sockaddr *address, socklen_t addr_len)
    {
      return negative_one_fails_with_errno<int>
        (::connect (sock, address, addr_len));
    }

    FILE *popen (const char *command, const char *type)
    {
      return nullptr_fails_with_errno<FILE> (::popen (command, type));
    }

    void pclose (FILE *stream)
    {
      return negative_one_fails_with_errno<void> (::pclose (stream));
    }

    size_t fread (void *ptr, size_t size, size_t nmemb, FILE *stream)
    {
      const size_t nread (::fread (ptr, size, nmemb, stream));
      if (nread < nmemb)
      {
        if (negative_one_fails_with_errno<int> (::ferror (stream)))
        {
          throw std::runtime_error ("could not read from FILE stream");
        }
        negative_one_fails_with_errno<int> (::feof (stream));
      }
      return nread;
    }
  }
}
