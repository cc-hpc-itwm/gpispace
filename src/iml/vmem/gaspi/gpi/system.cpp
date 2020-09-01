#include <iml/vmem/gaspi/gpi/system.hpp>

#include <util-generic/syscall.hpp>

namespace sys
{
  uint64_t get_total_memory_size ()
  {
    const long total_pages = fhg::util::syscall::sysconf (_SC_PHYS_PAGES);
    return total_pages * fhg::util::syscall::sysconf (_SC_PAGESIZE);
  }

  uint64_t get_avail_memory_size ()
  {
    const long avail_pages = fhg::util::syscall::sysconf (_SC_AVPHYS_PAGES);
    return avail_pages * fhg::util::syscall::sysconf (_SC_PAGESIZE);
  }
}
