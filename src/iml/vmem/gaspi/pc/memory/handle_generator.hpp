#pragma once

#include <vector>

#include <boost/noncopyable.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>
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

        gpi::pc::type::handle_t next();

      private:
        gpi::pc::type::size_t m_node_identifier;
        std::atomic<gpi::pc::type::size_t> m_counter;
      };
    }
  }
}
