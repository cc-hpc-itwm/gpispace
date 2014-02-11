// bernd.loerwald@itwm.fraunhofer.de

#include <unistd.h>

namespace fhg
{
  namespace syscall
  {
    void chdir (const char* path);
    void close (int fd);
    pid_t fork();
    pid_t setsid();
    void shutdown (int sockfd, int how);
  }
}
