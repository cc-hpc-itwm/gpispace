// bernd.loerwald@itwm.fraunhofer.de

#include <unistd.h>

namespace fhg
{
  namespace syscall
  {
    void close (int fd);
    pid_t fork();
    void shutdown (int sockfd, int how);
  }
}
