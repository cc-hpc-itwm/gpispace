#pragma once

#include <vector>

#include <boost/noncopyable.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>
#include <iml/vmem/gaspi/pc/type/segment_descriptor.hpp>
#include <iml/vmem/gaspi/pc/type/handle.hpp>

#include <atomic>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class handle_generator_t : boost::noncopyable
      {
      public:
        /** create a new handle_generator for a given node identifier.

            usually the rank can be used
         */

        explicit
        handle_generator_t (const gpi::pc::type::size_t identifier);

        /*
          generate a new handle for the given segment

          the handle ids are generated as follows:
          the four most significant bits indicate:

              0x0     - invalid alloc
              0x1     - gpi allocation
              0x2     - shared memory
              0x3     - gpu allocation
              0x4-0xf - reserved

          the remaining bits are assigned in an implementation specific way.
        */
        gpi::pc::type::handle_t
        next (const gpi::pc::type::segment::segment_type);

        void initialize_counter (const gpi::pc::type::segment::segment_type);
      private:
        gpi::pc::type::size_t m_node_identifier;
        std::vector<std::atomic<gpi::pc::type::size_t>> m_counter;
      };
    }
  }
}
