#ifndef GPI_SPACE_PC_PROTO_MEMORY_HPP
#define GPI_SPACE_PC_PROTO_MEMORY_HPP 1

#include <ostream>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/handle_descriptor.hpp>

namespace gpi
{
  namespace pc
  {
    namespace proto
    {
      namespace memory
      {
        struct alloc_t
        {
          gpi::pc::type::segment_id_t segment;
          gpi::pc::type::size_t size;
          gpi::pc::type::mode_t perm;
        };

        std::ostream & operator<<(std::ostream & os, const alloc_t & a)
        {
          os << "ALLOC of " << a.size << " byte(s) in segment "
             << a.segment << " with permissions " << a.perm;
          return os;
        }

        struct alloc_reply_t
        {
          gpi::pc::type::handle_id_t handle;
        };

        struct free_t
        {
          gpi::pc::type::handle_id_t handle;
        };

        struct free_reply_t
        {
          gpi::pc::type::error_t error;
        };

        struct list_t
        {
        };

        struct list_reply_t
        {
          gpi::pc::type::handle::list_t list;
        };

        struct memory_location_t
        {
          gpi::pc::type::handle_id_t handle;
          gpi::pc::type::offset_t    offset;
        };

        struct memcpy_t
        {
          memory_location_t dst;
          memory_location_t src;
          gpi::pc::type::size_t size;
          gpi::pc::type::queue_id_t queue;
        };

        struct memcpy_reply_t
        {
          gpi::pc::type::queue_id_t queue;
        };

        struct wait_t
        {
          gpi::pc::type::queue_id_t queue;
        };

        struct wait_reply_t
        {
          gpi::pc::type::error_t error;
        };
      }
    }
  }
}

#endif
