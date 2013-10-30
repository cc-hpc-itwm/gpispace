#ifndef GPI_SPACE_PC_MESSAGE_MESSAGE_HPP
#define GPI_SPACE_PC_MESSAGE_MESSAGE_HPP 1

#include <boost/variant.hpp>

// serialization
#include <boost/mpl/vector.hpp>

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
      struct header_t
      {
        header_t ()
          : length (0)
          , version (0x01)
          , flags (0)
          , seq (0)
          , ack (0)
          , checksum (0)
        {}

        uint32_t     length;
        uint8_t     version;
        uint8_t       flags;
        uint16_t        seq;
        uint16_t        ack;
        uint32_t   checksum;
      };

      typedef boost::variant< error::error_t
        , gpi::pc::proto::control::message_t
        , gpi::pc::proto::memory::message_t
        , gpi::pc::proto::segment::message_t
        > message_t;

      struct complete_message_t
      {
        header_t header;
        message_t  payload;
      };
    }
  }
}

#endif
