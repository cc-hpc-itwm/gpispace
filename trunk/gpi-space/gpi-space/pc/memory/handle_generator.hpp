#ifndef GPI_SPACE_PC_MEMORY_HANDLE_GENERATOR_HPP
#define GPI_SPACE_PC_MEMORY_HANDLE_GENERATOR_HPP 1

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/handle.hpp>

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
          the two most significant bits indicate:

                00   - invalid alloc
                01   - global alloc
                10   - local alloc
                11   - shared allocation

          the remaining bits are assigned in an implementation specific way.
        */
        gpi::pc::type::handle_t next (const gpi::pc::type::segment_id_t);
      private:
        explicit
        handle_generator_t (const gpi::pc::type::size_t identifier);

        gpi::pc::type::size_t increment ();

        static handle_generator_t * instance;

        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        mutable mutex_type m_mutex;
        gpi::pc::type::size_t m_node_identifier;
        gpi::pc::type::size_t m_counter;
      };
    }
  }
}

#endif
