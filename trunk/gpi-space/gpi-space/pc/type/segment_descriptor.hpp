#ifndef GPI_SPACE_PC_TYPE_SEGMENT_DESCRIPTOR_HPP
#define GPI_SPACE_PC_TYPE_SEGMENT_DESCRIPTOR_HPP 1

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/time_stamp.hpp>
#include <gpi-space/pc/type/generic_list.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      namespace segment
      {
        enum segment_type
          {
            SEG_INVAL = 0,
            SEG_GLOBAL,
            SEG_LOCAL,
            // the following is just a placeholder
            // shared segment ids are just >= SHARED
            SEG_SHARED,
          };

        struct traits
        {
          static bool is_valid (const gpi::pc::type::segment_id_t i)
          {
            return i != SEG_INVAL;
          }

          static bool is_global (const gpi::pc::type::segment_id_t i)
          {
            return i == SEG_GLOBAL;
          }
          static bool is_local (const gpi::pc::type::segment_id_t i)
          {
            return i == SEG_LOCAL;
          }
          static bool is_shared (const gpi::pc::type::segment_id_t i)
          {
            return i >= SEG_SHARED;
          }
        };

        struct descriptor_t
        {
          typedef traits traits_type;
          gpi::pc::type::segment_id_t id;
          gpi::pc::type::path_t path;
          gpi::pc::type::ref_count_t nref;
          gpi::pc::type::mode_t perm;
          gpi::pc::type::time_stamp_t ts;
        };

        typedef gpi::pc::type::generic_list_t<descriptor_t> list_t;
      }
    }
  }
}

#endif
