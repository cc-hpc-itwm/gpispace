#ifndef GPI_SPACE_PC_PROTO_SEGMENT_HPP
#define GPI_SPACE_PC_PROTO_SEGMENT_HPP 1

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/segment_descriptor.hpp>

namespace gpi
{
  namespace pc
  {
    namespace proto
    {
      namespace segment
      {
        struct attach_t
        {
          gpi::pc::type::path_t path;
          gpi::pc::type::mode_t perm;
        };

        struct attach_reply_t
        {
          union
          {
            gpi::pc::type::error_t error;
            gpi::pc::type::segment_id_t id;
          };
        };

        struct detach_t
        {
          gpi::pc::type::segment_id_t id;
        };

        struct detach_reply_t
        {
          gpi::pc::type::error_t error;
        };

        struct list_t
        {
        };

        struct list_reply_t
        {
          union
          {
            gpi::pc::type::error_t error;
            gpi::pc::type::segment::list_t list;
          };
        };
      }
    }
  }
}

#endif
