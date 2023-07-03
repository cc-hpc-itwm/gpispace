// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/syscall.hpp>

#include <util-generic/map_failed.hpp>
#include <util-generic/sig_err.hpp>
#include <util-generic/warning.hpp>

#include <boost/system/system_error.hpp>

#include <mutex>

#include <fcntl.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/mman.h>
#if FUNCTION_EXISTS_SIGNALFD
#include <sys/signalfd.h>
#endif
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>

namespace fhg
{
  namespace util
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
            throw ::boost::system::system_error
              (::boost::system::error_code (errno, ::boost::system::system_category()));
          }
          return rc;
        }

        template<typename T, typename R>
          T negative_one_fails_with_errno_ignore_sign_conversion
            (R rc, const char* reason)
        {
          if (rc == R (-1))
          {
            throw ::boost::system::system_error
              (::boost::system::error_code (errno, ::boost::system::system_category()));
          }
          return suppress_warning::sign_conversion<T> (rc, reason);
        }

        template<typename T>
        T *nullptr_fails_with_errno (T *rc)
        {
          if (rc == nullptr)
          {
            throw ::boost::system::system_error
              (::boost::system::error_code (errno, ::boost::system::system_category()));
          }
          return rc;
        }

        template<>
          void negative_one_fails_with_errno<void, int> (int rc)
        {
          if (rc == -1)
          {
            throw ::boost::system::system_error
              (::boost::system::error_code (errno, ::boost::system::system_category()));
          }
        }

        void* MAP_FAILED_fails_with_errno (void* rc)
        {
          if (rc == map_failed())
          {
            throw ::boost::system::system_error
              (::boost::system::error_code (errno, ::boost::system::system_category()));
          }
          return rc;
        }

        sighandler_t SIG_ERR_fails_with_errno (sighandler_t rc)
        {
          if (rc == sig_err())
          {
            throw ::boost::system::system_error
              (::boost::system::error_code (errno, ::boost::system::system_category()));
          }
          return rc;
        }

        template<typename R>
          void non_zero_is_error_code (R rc)
        {
          if (rc != R (0))
          {
            throw ::boost::system::system_error
              (::boost::system::error_code (rc, ::boost::system::system_category()));
          }
        }

        void non_zero_fails_with_gai_error (int rc)
        {
          if (rc)
          {
            throw std::runtime_error (gai_strerror (rc));
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

      void fcntl_setlkw (int fd, struct flock* info)
      {
        return negative_one_fails_with_errno<void> (::fcntl (fd, F_SETLKW, info));
      }

      void fdatasync (int fd)
      {
        return negative_one_fails_with_errno<void> (::fdatasync (fd));
      }

      char* fgets (char* str, int num, FILE* stream)
      {
        return nullptr_fails_with_errno<char> (::fgets (str, num, stream));
      }

      void flock (int fd, int operation)
      {
        return negative_one_fails_with_errno<void> (::flock (fd, operation));
      }

      void fcntl_setfl (int fd, int flags)
      {
        return negative_one_fails_with_errno<void> (::fcntl (fd, F_SETFL, flags));
      }

      pid_t fork()
      {
        return negative_one_fails_with_errno<pid_t> (::fork());
      }

      void ftruncate (int fd, off_t length)
      {
        return negative_one_fails_with_errno<void> (::ftruncate (fd, length));
      }

      struct stat fstat (int fd)
      {
        struct stat s;

        negative_one_fails_with_errno<void> (::fstat (fd, &s));

        return s;
      }

      char* getenv (const char* name)
      {
        return cannot_fail (::getenv (name));
      }

      pid_t getpid()
      {
        return cannot_fail (::getpid());
      }

      pid_t gettid()
      {
        return suppress_warning::shorten_64_to_32_with_check<pid_t>
          ( cannot_fail (::syscall (SYS_gettid))
          , "indirect system call returns a single type"
          );
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

      off_t lseek (int fd, off_t offset, int whence)
      {
        return negative_one_fails_with_errno<off_t> (::lseek (fd, offset, whence));
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
      void pipe (int pipefd[2], int flags)
      {
#if FUNCTION_EXISTS_PIPE2
        return negative_one_fails_with_errno<void> (::pipe2 (pipefd, flags));
#else
        if (flags != O_NONBLOCK)
        {
          throw std::invalid_argument ("flags != O_NONBLOCK");
        }
        pipe (pipefd);
        fcntl_setfl (pipefd[0], flags);
        fcntl_setfl (pipefd[1], flags);
#endif
      }

      void pthread_sigmask (int how, const sigset_t* set, sigset_t* oset)
      {
        return non_zero_is_error_code (::pthread_sigmask (how, set, oset));
      }

      void pthread_mutex_destroy (pthread_mutex_t* mutex)
      {
        return non_zero_is_error_code (::pthread_mutex_destroy (mutex));
      }

      void pthread_mutex_init
        (pthread_mutex_t* mutex, const pthread_mutexattr_t* attr)
      {
        return non_zero_is_error_code (::pthread_mutex_init (mutex, attr));
      }

      void pthread_mutex_lock (pthread_mutex_t* mutex)
      {
        return non_zero_is_error_code (::pthread_mutex_lock (mutex));
      }

      void pthread_mutex_unlock (pthread_mutex_t* mutex)
      {
        return non_zero_is_error_code (::pthread_mutex_unlock (mutex));
      }

      size_t read (int fd, void* buf, size_t count)
      {
        return negative_one_fails_with_errno_ignore_sign_conversion<size_t>
          (::read (fd, buf, count), "read never returns negative except for -1");
      }

      int select (int nfds, fd_set* read, fd_set* write, fd_set* except, timeval* timeout)
      {
        return negative_one_fails_with_errno<int>
          (::select (nfds, read, write, except, timeout));
      }

      void setenv (char const* name, char const* value, int overwrite)
      {
        return negative_one_fails_with_errno<void>
          (::setenv (name, value, overwrite));
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
        return negative_one_fails_with_errno<void>
          (::sigaddset (set, signum));
      }

      void sigemptyset (sigset_t* set)
      {
        return negative_one_fails_with_errno<void>
          (::sigemptyset (set));
      }

      sighandler_t signal (int signum, sighandler_t handler)
      {
        return SIG_ERR_fails_with_errno (::signal (signum, handler));
      }

#if FUNCTION_EXISTS_SIGNALFD
      int signalfd (int fd, sigset_t const* mask, int flags)
      {
        return negative_one_fails_with_errno<int>
          (::signalfd (fd, mask, flags));
      }
#endif

      void sigprocmask (int how, sigset_t const* sig, sigset_t* old)
      {
        return negative_one_fails_with_errno<void>
          (::sigprocmask (how, sig, old));
      }

      int sigtimedwait
        (sigset_t const* set, siginfo_t* info, timespec const* timeout)
      {
        return negative_one_fails_with_errno<int>
          (::sigtimedwait (set, info, timeout));
      }

      int sigwaitinfo (sigset_t const* set, siginfo_t* info)
      {
        return negative_one_fails_with_errno<int>
          (::sigwaitinfo (set, info));
      }

      int socket (int domain, int type, int protocol)
      {
        return negative_one_fails_with_errno<int> (::socket (domain, type, protocol));
      }

      void stat (const char* path, struct stat* buf)
      {
        return negative_one_fails_with_errno<void> (::stat (path, buf));
      }

      long sysconf (int name)
      {
        errno = 0;
        long const ret  (::sysconf (name));
        if (ret == -1 && errno == EINVAL)
        {
          throw ::boost::system::system_error
            (::boost::system::error_code (EINVAL, ::boost::system::system_category()));
        }
        return ret;
      }

      void unlink (const char* pathname)
      {
        return negative_one_fails_with_errno<void> (::unlink (pathname));
      }

      void unsetenv (char const* name)
      {
        return negative_one_fails_with_errno<void> (::unsetenv (name));
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

      size_t write (int fd, const void* buf, size_t count)
      {
        return negative_one_fails_with_errno_ignore_sign_conversion<size_t>
          (::write (fd, buf, count), "write never returns negative except for -1");
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

      int pclose (FILE *stream)
      {
        return negative_one_fails_with_errno<int> (::pclose (stream));
      }

      void freeaddrinfo (::addrinfo* info)
      {
        ::freeaddrinfo (info);
      }
      addrinfo_ptr getaddrinfo
        (char const* node, char const* service, ::addrinfo const* hints)
      {
        ::addrinfo* res (nullptr);
        non_zero_fails_with_gai_error
          (::getaddrinfo (node, service, hints, &res));
        return addrinfo_ptr (res, &freeaddrinfo);
      }

      DIR* opendir (char const* name)
      {
        return nullptr_fails_with_errno<DIR> (::opendir (name));
      }
      void closedir (DIR* dir)
      {
        return negative_one_fails_with_errno<void> (::closedir (dir));
      }
      int dirfd (DIR* dir)
      {
        return negative_one_fails_with_errno<int> (::dirfd (dir));
      }

      namespace
      {
        void dlerror (std::string const& name)
        {
          if (auto error = ::dlerror())
          {
            throw std::runtime_error (name + ": " + error);
          }
        }

        struct check_dlerror
        {
          check_dlerror (std::string name)
            : _lock (_guard)
            , _name (std::move (name))
          {
            dlerror (_name);
          }
          template<typename Ret> Ret check (Ret value) const
          {
            dlerror (_name);
            return value;
          }

          //! \note dlerror() is specified to be not threadsafe. In
          //! fact, it uses a single static buffer to print the error
          //! message in. Thus, sequentialize all functions that call
          //! dlerror.
          static std::mutex _guard;
          std::lock_guard<std::mutex> const _lock;
          std::string _name;
        };

        std::mutex check_dlerror::_guard;
      }

      void* dlopen (char const* filename, int flag)
      {
        check_dlerror const _ ("dlopen");
        return _.check (::dlopen (filename, flag));
      }
      void* dlsym (void* handle, char const* symbol)
      {
        check_dlerror const _ ("dlsym");
        return _.check (::dlsym (handle, symbol));
      }
      void dlclose (void* handle)
      {
        check_dlerror const _ ("dlclose");
        _.check (::dlclose (handle));
      }
      Dl_info dladdr (void* addr)
      {
        check_dlerror const _ ("dladdr");
        Dl_info info;
        _.check (::dladdr (addr, &info));
        return info;
      }

      void mlock (void const* addr, size_t length)
      {
        return negative_one_fails_with_errno<void> (::mlock (addr, length));
      }
      void munlock (void const* addr, size_t length)
      {
        return negative_one_fails_with_errno<void> (::munlock (addr, length));
      }
    }
  }
}
