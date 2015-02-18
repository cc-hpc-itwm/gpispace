#pragma once

#include <ostream>
#include <boost/shared_ptr.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/handle.hpp>
#include <gpi-space/pc/memory/memory_area.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      /*
       Describes a scheduled/ongoing memory transfer.

          Pointers to the affected memory areas are stored to prohibit
          deletion of those.

          A memory transfer is fully specified by:
            - dst memory area, dst handle, dst offset
            - src memory area, src handle, src offset
            - transfer size
       */
      class memory_transfer_t
      {
      public:
        memory_transfer_t ()
          : amount (0)
          , queue (0)
        {}

        typedef boost::shared_ptr<gpi::pc::memory::area_t> area_ptr;

        area_ptr dst_area;
        gpi::pc::type::memory_location_t dst_location;

        area_ptr src_area;
        gpi::pc::type::memory_location_t src_location;

        gpi::pc::type::size_t amount;
        gpi::pc::type::queue_id_t queue;
      };
    }
  }
}
