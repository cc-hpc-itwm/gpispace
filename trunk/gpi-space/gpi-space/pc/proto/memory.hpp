#ifndef GPI_SPACE_PC_PROTO_MEMORY_HPP
#define GPI_SPACE_PC_PROTO_MEMORY_HPP 1

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

        struct alloc_reply_t
        {
          union
          {
            gpi::pc::type::error_t error;
            gpi::pc::type::handle_id_t handle;
          };
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
          union
          {
            gpi::pc::type::error_t error;
            gpi::pc::type::handle::list_t list;
          };
        };

        union message_t
        {
          struct
          {
            union
            {
              memory::alloc_t       req;
              memory::alloc_reply_t rpl;
            };
          } alloc;

          struct
          {
            union
            {
              memory::free_t req;
              memory::free_reply_t rpl;
            };
          } free;

          struct
          {
            union
            {
              memory::list_t req;
              memory::list_reply_t rpl;
            };
          } list;
        };
      }
    }
  }
}

#endif
