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

            segment_attach, segment_attach_reply,
            segment_detach, segment_detach_reply,
            segment_list,   segment_list_reply,

            memory_alloc,  memory_alloc_reply,
            memory_free,   memory_free_reply,
            memory_list,   memory_list_reply,
            memory_memcpy, memory_memcpy_reply,
            memory_wait,   memory_wait_reply,
          };
      }

      struct message_t
      {
        message::type type;
        gpi::pc::type::size_t length;
        char payload[0];

        template <typename T>
        T* as() { return (T*)&payload; }

        template <typename T>
        const T* as() const { return (const T*)&payload; }
      };
    }
  }
}

#endif
