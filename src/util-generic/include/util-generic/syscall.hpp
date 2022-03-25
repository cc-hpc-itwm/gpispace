// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>

#include <cstdio>
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace fhg
{
  namespace util
  {
    namespace syscall
    {
      int accept (int sockfd, struct sockaddr* addr, socklen_t* addrlen);
      void bind (int sockfd, const struct sockaddr* addr, socklen_t addrlen);
      void chdir (const char* path);
      void chmod (const char* path, mode_t mode);
      void close (int fd);
      int dup (int oldfd);
      int dup (int oldfd, int newfd);
      [[noreturn]] void execve (const char* filename, char* const argv[], char* const envp[]);
      void fcntl_setlkw (int fd, struct flock*);
      void fdatasync (int fd);
      char* fgets (char* str, int num, FILE* stream);
      void flock (int fd, int operation);
      void fcntl_setfl (int fd, int flags);
      pid_t fork();
      void ftruncate (int fd, off_t length);
      struct stat fstat (int fd);
      char* getenv (const char* name);
      pid_t getpid();
      pid_t gettid();
      uid_t getuid();
      void kill (pid_t pid, int sig);
      void listen (int sockfd, int backlog);
      off_t lseek (int fd, off_t, int whence);
      void* mmap (void* addr, size_t length, int prot, int flags, int fd, off_t offset);
      void mkfifo (const char* pathname, mode_t mode);
      void munmap (void* addr, size_t length);
      int open (const char* pathname, int flags);
      int open (const char* pathname, int flags, mode_t mode);
      void pipe (int pipefd[2]);
      void pipe (int pipefd[2], int flags);
      void pthread_sigmask (int how, const sigset_t* set, sigset_t* oset);
      void pthread_mutex_destroy (pthread_mutex_t*);
      void pthread_mutex_init (pthread_mutex_t*, const pthread_mutexattr_t*);
      void pthread_mutex_lock (pthread_mutex_t*);
      void pthread_mutex_unlock (pthread_mutex_t*);
      size_t read (int fd, void* buf, size_t count);
      int select (int nfds, fd_set* read, fd_set* write, fd_set* except, timeval* timeout);
      void setenv (char const* name, char const* value, int overwrite);
      pid_t setsid();
      void setsockopt (int sockfd, int level, int optname, const void* optval, socklen_t optlen);
      int shm_open (const char* name, int oflag, mode_t mode);
      void shm_unlink (const char* name);
      void shutdown (int sockfd, int how);
      void sigaction (int signum, const struct sigaction* act, struct sigaction* oldact);
      void sigaddset (sigset_t* set, int signum);
      void sigemptyset (sigset_t* set);
      sighandler_t signal (int signum, sighandler_t handler);
      int signalfd (int fd, sigset_t const*, int flags);
      void sigprocmask (int how, sigset_t const*, sigset_t* old);
      int sigtimedwait (sigset_t const* set, siginfo_t* info, timespec const* timeout);
      int sigwaitinfo (sigset_t const* set, siginfo_t* info);
      int socket (int domain, int type, int protocol);
      void stat (const char* path, struct stat* buf);
      long sysconf (int name);
      void unlink (const char* pathname);
      void unsetenv (char const*);
      pid_t wait (pid_t pid, int* status, int options, struct rusage* rusage);
      pid_t waitpid (pid_t pid, int* status, int options);
      size_t write (int fd, const void* buf, size_t count);
      int connect (int sock, const struct sockaddr *address, socklen_t addr_len);
      FILE *popen (const char *command, const char *type);
      int pclose (FILE *stream);

      void freeaddrinfo (::addrinfo*);
      using addrinfo_ptr
        = std::unique_ptr<::addrinfo, decltype (&freeaddrinfo)>;
      addrinfo_ptr getaddrinfo
        (char const* node, char const* service, ::addrinfo const* hints);

      DIR* opendir (char const* name);
      void closedir (DIR*);
      int dirfd (DIR*);

      void* dlopen (char const* filename, int flag);
      void* dlsym (void* handle, char const* symbol);
      void dlclose (void* handle);
      Dl_info dladdr (void* addr);

      void mlock (void const* addr, size_t length);
      void munlock (void const* addr, size_t length);
    }
  }
}
