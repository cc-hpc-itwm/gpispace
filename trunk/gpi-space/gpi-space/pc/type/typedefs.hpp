#ifndef GPI_SPACE_PC_TYPE_DEFINITIONS_HPP
#define GPI_SPACE_PC_TYPE_DEFINITIONS_HPP 1

#include <inttypes.h>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      typedef uint64_t size_t;
      typedef uint64_t offset_t;
      typedef uint64_t id_t;
      typedef uint64_t ref_count_t;
      typedef id_t segment_id_t;
      typedef id_t handle_id_t;
      typedef uint32_t mode_t; // r(4) u(4) g(4) o(4)
      typedef uint64_t time_t;
    }
  }
}

#endif
