#ifndef GPI_SPACE_PC_MESSAGE_MESSAGE_HPP
#define GPI_SPACE_PC_MESSAGE_MESSAGE_HPP 1

#include <boost/variant.hpp>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/proto/error.hpp>
#include <gpi-space/pc/proto/memory.hpp>
#include <gpi-space/pc/proto/segment.hpp>
#include <gpi-space/pc/proto/control.hpp>

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

      struct header_t
      {
        header_t ()
          : length (0)
          , version (0x01)
          , type (0)
          , flags (0)
        {}

        uint32_t     length;
        uint8_t     version;
        uint8_t        type;
        uint16_t      flags;
      };

      typedef boost::variant< error::error_t
                            , memory::alloc_t
                            , memory::alloc_reply_t
                            , memory::free_t
                            , memory::list_t
                            , memory::list_reply_t
                            , memory::memcpy_t
                            , memory::memcpy_reply_t
                            , memory::wait_t
                            , memory::wait_reply_t

                            , segment::register_t
                            , segment::register_reply_t
                            , segment::unregister_t
                            , segment::attach_t
                            , segment::detach_t
                            , segment::list_t
                            , segment::list_reply_t

                            > message_t;
    }
  }
}

#endif
