#include <stdlib.h>
#include <malloc.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>

int main (int ac, char **av)
{
  if (ac < 2)
  {
    fprintf (stderr, "usage: %s #bytes\n", av [0]);
    return 1;
  }

  uint64_t size = 0;
  if (1 != sscanf (av [1], "%lu", &size))
  {
    fprintf (stderr, "invalid number of bytes: %s\n", av [1]);
    return 1;
  }

  fprintf (stderr, "allocating %lu bytes...", size);
  fflush (stderr);

  void *memory (memalign (sysconf (_SC_PAGESIZE), size));
  if (0 == memory)
  {
    fprintf (stderr, "failed: memalign: %s\n", strerror (errno));
    return 2;
  }
  else
  {
    fprintf (stderr, "%p\n", memory);
  }

  fprintf (stderr, "performing fork test (before mlock)...");
  fflush (stderr);

  if (system ("echo forked before mlock") == -1)
  {
    fprintf (stderr, "fork failed: %s\n", strerror (errno));
    return 4;
  }

  fprintf (stderr, "locking memory...");
  fflush (stderr);

  if (mlock (memory, size) != 0)
  {
    fprintf (stderr, "failed: mlock: %s\n", strerror (errno));
    return 3;
  }
  else
  {
    fprintf (stderr, "ok\n");
  }

  fprintf (stderr, "performing fork test (after mlock)...");

  if (system ("echo forked after mlock") == -1)
  {
    fprintf (stderr, "failed: %s\n", strerror (errno));
    return 4;
  }

  free (memory);

  return EXIT_SUCCESS;
}
