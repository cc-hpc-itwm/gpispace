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
        enum
          {
            GLOBAL = 0,
            LOCAL = 1,
          };

        struct descriptor_t
        {
          gpi::pc::type::id_t id; // either GLOBAL, LOCAL or identifier
          gpi::pc::type::mode_t perm;
          gpi::pc::type::time_stamp_t ts;
        };

        typedef gpi::pc::type::generic_list_t<descriptor_t> list_t;
      }
    }
  }
}

#endif
