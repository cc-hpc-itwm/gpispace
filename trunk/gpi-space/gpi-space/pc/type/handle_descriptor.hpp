#ifndef GPI_SPACE_PC_TYPE_HANDLE_DESCRIPTOR_HPP
#define GPI_SPACE_PC_TYPE_HANDLE_DESCRIPTOR_HPP 1

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/time_stamp.hpp>
#include <gpi-space/pc/type/generic_list.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      namespace handle
      {
        struct traits
        {
          static bool is_null (const gpi::pc::type::handle_id_t i)
          {
            return 0 == i;
          }
        };

        struct descriptor_t
        {
          gpi::pc::type::handle_id_t id;
          gpi::pc::type::segment_id_t segment;
          gpi::pc::type::offset_t offset;
          gpi::pc::type::size_t size;
          gpi::pc::type::mode_t perm;
          gpi::pc::type::time_stamp_t ts;
        };

        typedef gpi::pc::type::generic_list_t<descriptor_t> list_t;
      }
    }
  }
}

#endif
