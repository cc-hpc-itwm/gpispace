#ifndef GPI_SPACE_PC_PROTO_ALLOC_HPP
#define GPI_SPACE_PC_PROTO_ALLOC_HPP 1

#include <gpi-space/pc/type/typedefs.hpp>

namespace gpi
{
  namespace pc
  {
    namespace proto
    {
      namespace alloc
      {
        struct request_t
        {
          gpi::pc::type::segment_id_t segment;
          gpi::pc::type::size_t size;
          gpi::pc::type::mode_t perm;
        };

        struct reply_t
        {
          gpi::pc::type::handle_id_t handle;
        };
      }
    }
  }
}

#endif
