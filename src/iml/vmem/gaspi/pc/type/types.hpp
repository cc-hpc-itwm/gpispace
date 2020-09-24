#pragma once

#include <inttypes.h>
#include <limits.h>
#include <string>

namespace gpi
{
  namespace pc
  {
    enum class is_global
    {
      no,
      yes,
    };

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

      using memcpy_id_t = id_t;

      typedef uint32_t error_t;
      typedef uint16_t mode_t;
      using flags_t = is_global;
      typedef std::string path_t;
      typedef std::string name_t;

      struct range_t
      {
        handle_id_t handle;
        offset_t offset;
        size_t size;

        range_t ( handle_id_t handle_
                , offset_t offset_
                , size_t size_
                )
          : handle (handle_)
          , offset (offset_)
          , size (size_)
        {}
      };
    }
  }
}
