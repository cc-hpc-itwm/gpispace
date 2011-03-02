#ifndef GPI_SPACE_PC_MEMORY_HANDLE_GENERATOR_HPP
#define GPI_SPACE_PC_MEMORY_HANDLE_GENERATOR_HPP 1

#include <vector>

#include <boost/noncopyable.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/segment_descriptor.hpp>
#include <gpi-space/pc/type/handle.hpp>
#include <gpi-space/pc/type/counter.hpp>

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
        static void create (const gpi::pc::type::size_t node_identifier);
        static handle_generator_t & get ();
        static void destroy ();

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
      private:
        typedef boost::shared_ptr<gpi::pc::type::counter_t> counter_ptr;

        explicit
        handle_generator_t (const gpi::pc::type::size_t identifier);

        static boost::shared_ptr<handle_generator_t> instance;
        gpi::pc::type::size_t m_node_identifier;
        std::vector<counter_ptr> m_counter;
      };
    }
  }
}

#endif
