#ifndef GPI_SPACE_PC_MEMORY_MEMORY_AREA_HPP
#define GPI_SPACE_PC_MEMORY_MEMORY_AREA_HPP 1

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/segment/segment.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class area_t : boost::noncopyable
      {
      public:
        typedef boost::shared_ptr<gpi::pc::segment::segment_t> segment_ptr;

        virtual ~area_t ();

        virtual gpi::pc::type::handle_id_t alloc (const gpi::pc::type::size_t size) = 0;
        virtual void free (const gpi::pc::type::handle_id_t hdl) = 0;
        virtual gpi::pc::type::size_t total_mem_size () = 0;
        virtual gpi::pc::type::size_t free_mem_size () = 0;
      protected:
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        mutable mutex_type m_mutex;
      };

      class shared_area_t : public area_t
      {
      private:
        segment_ptr m_segment;
      };

      class gpi_area_t : public area_t
      {
      private:
        segment_ptr m_segment_global;
        segment_ptr m_segment_local;
      };
    }
  }
}

#endif
