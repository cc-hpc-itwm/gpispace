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
        {}

        uint32_t     length;
      };

      typedef boost::variant< error::error_t
        , gpi::pc::proto::control::message_t
        , gpi::pc::proto::memory::message_t
        , gpi::pc::proto::segment::message_t
        > message_t;
    }
  }
}

#endif
