// bernd.loerwald@itwm.fraunhofer.de

#include <unistd.h>
#include <sys/types.h>

namespace fhg
{
  namespace syscall
  {
    void chdir (const char* path);
    void close (int fd);
    int dup (int oldfd);
    int dup (int olfd, int newfd);
    int dup (int olfd, int newfd, int flags);
    pid_t fork();
    int open (const char* pathname, int flags);
    int open (const char* pathname, int flags, mode_t mode);
    pid_t setsid();
    void shutdown (int sockfd, int how);
  }
}
