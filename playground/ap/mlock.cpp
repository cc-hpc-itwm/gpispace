#include <stdlib.h>
#include <malloc.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#include <sstream>
#include <iostream>

int main (int ac, char **av)
{
  if (ac < 2)
  {
    std::cerr << "usage: " << av[0] << " #bytes" << std::endl;
    return 1;
  }

  uint64_t size = 0;
  {
    std::stringstream sstr(av[1]);
    sstr >> size;
  }

  std::cerr << "allocating " << size << " bytes..." << std::endl;

  void *memory (memalign(sysconf(_SC_PAGESIZE), size));
  if (0 == memory)
  {
    std::cerr << "failed: memalign: " << strerror(errno) << std::endl;
    return 2;
  }

  std::cerr << "locking memory..." << std::endl;

  if (mlock(memory, size) != 0)
  {
    std::cerr << "failed: mlock: " << strerror(errno) << std::endl;
    return 3;
  }

  free (memory);

  return EXIT_SUCCESS;
}
