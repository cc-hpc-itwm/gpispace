#ifndef GPI_SPACE_PC_TYPE_TIMESTAMP_HPP
#define GPI_SPACE_PC_TYPE_TIMESTAMP_HPP 1

#include <gpi-space/pc/type/typedefs.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      struct time_stamp_t
      {
        gpi::pc::type::time_t created;
        gpi::pc::type::time_t modified;
        gpi::pc::type::time_t accessed;
      };
    }
  }
}

#endif
