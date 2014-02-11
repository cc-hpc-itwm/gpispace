// bernd.loerwald@itwm.fraunhofer.de

#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

namespace fhg
{
  namespace syscall
  {
    void chdir (const char* path);
    void chmod (const char* path, mode_t mode);
    void close (int fd);
    int dup (int oldfd);
    int dup (int olfd, int newfd);
    int dup (int olfd, int newfd, int flags);
    void execve (const char* filename, char* const argv[], char* const envp[]);
    pid_t fork();
    void kill (pid_t pid, int sig);
    int open (const char* pathname, int flags);
    int open (const char* pathname, int flags, mode_t mode);
    pid_t setsid();
    void shutdown (int sockfd, int how);
    void unlink (const char* pathname);
    pid_t wait (pid_t pid, int* status, int options, struct rusage* rusage);
  }
}
