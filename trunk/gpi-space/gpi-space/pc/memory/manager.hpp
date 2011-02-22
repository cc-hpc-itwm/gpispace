#ifndef GPI_SPACE_PC_MEMORY_MANAGER_HPP
#define GPI_SPACE_PC_MEMORY_MANAGER_HPP 1

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <gpi-space/pc/segment/manager.hpp>
#include <gpi-space/pc/memory/memory_area.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class manager_t : boost::noncopyable
      {
      public:
        explicit
        manager_t ( const gpi::pc::type::id_t ident
                  , gpi::pc::segment::manager_t & segment_mgr
                  );
        ~manager_t ();

        void add_area_for_segment (const gpi::pc::type::segment_id_t seg);
        void del_area_for_segment (const gpi::pc::type::segment_id_t seg);

        gpi::pc::type::handle_id_t alloc ( const gpi::pc::type::process_id_t proc_id
                                         , const gpi::pc::type::segment_id_t seg_id
                                         , const gpi::pc::type::size_t size
                                         , const std::string & name
                                         , const gpi::pc::type::flags_t flags
                                         );
        void free (const gpi::pc::type::handle_id_t hdl);
        void garbage_collect () {}
        void garbage_collect (const gpi::pc::type::process_id_t proc_id) {}
        void list_allocations( const gpi::pc::type::segment_id_t seg
                             , gpi::pc::type::handle::list_t & l
                             ) const;
        void list_allocations(gpi::pc::type::handle::list_t & l) const;
      private:
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
        typedef boost::shared_ptr<area_t> area_ptr;
        typedef boost::unordered_map< gpi::pc::type::segment_id_t
                                    , area_ptr
                                    > area_map_t;
        typedef boost::unordered_map< gpi::pc::type::handle_id_t
                                    , gpi::pc::type::segment_id_t
                                    > handle_to_segment_t;

        mutable mutex_type m_mutex;
        gpi::pc::type::id_t m_ident;
        gpi::pc::segment::manager_t & m_segment_mgr;
        area_map_t m_areas;
        handle_to_segment_t m_handle_to_segment;
      };
    }
  }
}

#endif
