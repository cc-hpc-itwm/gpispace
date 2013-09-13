#include "program_info.h"

#include <errno.h>
#include <unistd.h>

int fhg_get_executable_path (char *path, size_t len)
{
  int rc;

#ifdef __linux
  if (0 == len)
  {
    errno = EINVAL;
    return -1;
  }

  rc = readlink ("/proc/self/exe", path, len - 1);
  if (rc < 0)
  {
    return -1;
  }
  else
  {
    path [rc] = '\0';
  }

  return 0;
#else
# error "please implement 'fhg_get_executable_path (path, len)' for your OS"
#endif
}
