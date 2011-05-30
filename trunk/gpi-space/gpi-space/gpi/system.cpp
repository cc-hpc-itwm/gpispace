#include "system.hpp"

#include <unistd.h> // getpagesize, sysconf

namespace sys
{
  uint64_t get_total_memory_size ()
  {
    const long total_pages = sysconf(_SC_PHYS_PAGES);
    return total_pages * getpagesize();
  }

  uint64_t get_avail_memory_size ()
  {
    const long avail_pages = sysconf(_SC_AVPHYS_PAGES);
    return avail_pages * getpagesize();
  }
}
