#pragma once

#include <boost/variant.hpp>

// serialization
#include <boost/mpl/vector.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/variant.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>
#include <iml/vmem/gaspi/pc/proto/error.hpp>
#include <iml/vmem/gaspi/pc/proto/memory.hpp>
#include <iml/vmem/gaspi/pc/proto/segment.hpp>

#include <unordered_set>

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

        void clear()
        {
          length = 0;
        }

        uint32_t     length;
      };

      struct existing_segments
      {
        template<typename Archive>
          void serialize (Archive&, unsigned int)
        {
        }
      };
      struct existing_allocations
      {
        type::segment_id_t segment;
        template<typename Archive>
          void serialize (Archive& ar, unsigned int)
        {
          ar & segment;
        }
      };

      typedef boost::variant< error::error_t
        , gpi::pc::proto::memory::message_t
        , gpi::pc::proto::segment::message_t
                            , existing_segments
                            , existing_allocations
                            , std::unordered_set<type::segment_id_t>
                            , std::unordered_set<type::handle_id_t>
        > message_t;
    }
  }
}
