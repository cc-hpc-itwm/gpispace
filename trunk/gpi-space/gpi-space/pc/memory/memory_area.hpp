#ifndef GPI_SPACE_PC_MEMORY_MEMORY_AREA_HPP
#define GPI_SPACE_PC_MEMORY_MEMORY_AREA_HPP 1

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include <mmgr/dtmmgr.h>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/handle_descriptor.hpp>
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

        explicit
        area_t (const segment_ptr & seg);
        virtual ~area_t ();

        void alloc ( const gpi::pc::type::size_t size
                   , const std::string & name
                   , const gpi::pc::type::flags_t flags = gpi::pc::type::handle::F_NONE
                   );
        void free (const gpi::pc::type::handle_id_t hdl);
        gpi::pc::type::size_t total_mem_size () const;
        gpi::pc::type::size_t free_mem_size () const;
        gpi::pc::type::size_t used_mem_size () const;
      protected:
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
        typedef boost::unordered_map< gpi::pc::type::handle_id_t
                                    , gpi::pc::type::handle::descriptor_t
                                    > handle_descriptor_map_t;

        virtual void free_hook (const gpi::pc::type::handle_id_t) {}
        virtual void alloc_hook ( const gpi::pc::type::handle_id_t
                                , const::gpi::pc::type::size_t
                                , const int arena
                                ) {}

        mutable mutex_type m_mutex;
        segment_ptr m_segment;
        DTmmgr_t m_mmgr;
        handle_descriptor_map_t m_handles;
      };
    }
  }
}

#endif
