// bernd.loerwald@itwm.fraunhofer.de

#include <signal.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace fhg
{
  namespace syscall
  {
    int accept (int sockfd, struct sockaddr* addr, socklen_t* addrlen);
    void bind (int sockfd, const struct sockaddr* addr, socklen_t addrlen);
    void chdir (const char* path);
    void chmod (const char* path, mode_t mode);
    void close (int fd);
    int dup (int oldfd);
    int dup (int olfd, int newfd);
    [[noreturn]] void execve (const char* filename, char* const argv[], char* const envp[]);
    [[noreturn]] void execvp (const char* filename, char* const argv[]);
    [[noreturn]] void execvpe (const char* filename, char* const argv[], char *const envp[]);
    pid_t fork();
    void ftruncate (int fd, off_t length);
    char* getenv (const char* name);
    pid_t getpid();
    uid_t getuid();
    void kill (pid_t pid, int sig);
    void listen (int sockfd, int backlog);
    void* mmap (void* addr, size_t length, int prot, int flags, int fd, off_t offset);
    void mkfifo (const char* pathname, mode_t mode);
    void munmap (void* addr, size_t length);
    int open (const char* pathname, int flags);
    int open (const char* pathname, int flags, mode_t mode);
    void pipe (int pipefd[2]);
    void pthread_sigmask (int how, const sigset_t* set, sigset_t* oset);
    ssize_t read (int fd, void* buf, size_t count);
    pid_t setsid();
    void setsockopt (int sockfd, int level, int optname, const void* optval, socklen_t optlen);
    int shm_open (const char* name, int oflag, mode_t mode);
    void shm_unlink (const char* name);
    void shutdown (int sockfd, int how);
    void sigaction (int signum, const struct sigaction* act, struct sigaction* oldact);
    void sigaddset (sigset_t* set, int signum);
    void sigemptyset (sigset_t* set);
    sighandler_t signal (int signum, sighandler_t handler);
    int socket (int domain, int type, int protocol);
    void stat (const char* path, struct stat* buf);
    void unlink (const char* pathname);
    pid_t wait (pid_t pid, int* status, int options, struct rusage* rusage);
    pid_t waitpid (pid_t pid, int* status, int options);
    ssize_t write (int fd, const void* buf, size_t count);
    int connect (int sock, const struct sockaddr *address, socklen_t addr_len);
  }
}
