#ifndef GPI_SPACE_PC_TYPE_DEFINITIONS_HPP
#define GPI_SPACE_PC_TYPE_DEFINITIONS_HPP 1

#include <inttypes.h>
#include <limits.h>
#include <string>

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
      typedef ::time_t time_t;
      typedef id_t segment_id_t;
      typedef id_t handle_id_t;
      typedef id_t queue_id_t;
      typedef id_t process_id_t;
      typedef id_t rank_t;

      typedef uint32_t error_t;
      typedef uint16_t mode_t;
      typedef uint16_t flags_t;
      typedef std::string path_t;
      typedef std::string name_t;
    }
  }

  namespace flag
  {
    inline
    bool is_set (const pc::type::flags_t f, const pc::type::flags_t mask)
    {
      return (f & mask);
    }

    inline
    void set (pc::type::flags_t & f, const pc::type::flags_t mask)
    {
      f |= mask;
    }

    inline
    void clear (pc::type::flags_t & f, const pc::type::flags_t mask)
    {
      f &= ~mask;
    }

    inline
    void unset (pc::type::flags_t & f, const pc::type::flags_t mask)
    {
      f &= ~mask;
    }
  }
}

#endif
