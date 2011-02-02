#ifndef GPI_SPACE_PC_MESSAGE_MESSAGE_HPP
#define GPI_SPACE_PC_MESSAGE_MESSAGE_HPP 1

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/proto/error.hpp>
#include <gpi-space/pc/proto/memory.hpp>
#include <gpi-space/pc/proto/segment.hpp>

namespace gpi
{
  namespace pc
  {
    namespace proto
    {
      namespace message
      {
        enum type
          {
            error,

            segment_attach,
            segment_detach,
            segment_list,

            memory_alloc,
            memory_free,
            memory_list,
          };
      }

      struct message_t
      {
        gpi::pc::type::size_t length;
        message::type type;

        union
        {
          error::message_t error;
          memory::message_t memory;
          segment::message_t segment;
        };
      };
    }
  }
}

#endif
