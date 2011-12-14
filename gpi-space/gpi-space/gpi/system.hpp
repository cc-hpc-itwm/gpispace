#ifndef GPI_SPACE_GPI_API_SYSTEM_HPP
#define GPI_SPACE_GPI_API_SYSTEM_HPP 1

#include <inttypes.h>

namespace sys
{
  uint64_t get_total_memory_size ();
  uint64_t get_avail_memory_size ();
}

#endif
