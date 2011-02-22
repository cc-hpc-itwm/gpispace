#ifndef GPI_SPACE_PC_MEMORY_MANAGER_HPP
#define GPI_SPACE_PC_MEMORY_MANAGER_HPP 1

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/signals2.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/counter.hpp>
#include <gpi-space/pc/segment/segment.hpp>
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
        typedef boost::shared_ptr <gpi::pc::segment::segment_t>
                memory_ptr;

        // signals
        typedef boost::signals2::signal
            <void (const gpi::pc::type::segment_id_t)>
               memory_signal_t;
        memory_signal_t memory_added;
        memory_signal_t memory_removed;

        explicit
        manager_t (const gpi::pc::type::id_t ident);
        ~manager_t ();

        void clear ();

        gpi::pc::type::segment_id_t
        register_memory( const gpi::pc::type::process_id_t creator
                       , const std::string & name
                       , const gpi::pc::type::size_t size
                       , const gpi::pc::type::flags_t flags
                       );
        void unregister_memory (const gpi::pc::type::segment_id_t);
        void list_memory (gpi::pc::type::segment::list_t &) const;

        gpi::pc::type::size_t
        increment_refcount (const gpi::pc::type::segment_id_t seg);

        gpi::pc::type::size_t
        decrement_refcount (const gpi::pc::type::segment_id_t seg);

        void add_special_memory ( std::string const & name
                                , const gpi::pc::type::segment_id_t id
                                , const gpi::pc::type::size_t size
                                , void *ptr
                                );
        memory_ptr get_memory (const gpi::pc::type::segment_id_t);
        memory_ptr operator [] (const gpi::pc::type::segment_id_t seg_id)
        {
          return get_memory (seg_id);
        }

        gpi::pc::type::handle_id_t
        alloc ( const gpi::pc::type::process_id_t proc_id
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
        typedef boost::unordered_map<gpi::pc::type::segment_id_t, memory_ptr> segment_map_t;
        typedef boost::unordered_map< gpi::pc::type::segment_id_t
                                    , area_ptr
                                    > area_map_t;
        typedef boost::unordered_map< gpi::pc::type::handle_id_t
                                    , gpi::pc::type::segment_id_t
                                    > handle_to_segment_t;

        mutable mutex_type m_mutex;
        gpi::pc::type::id_t m_ident;
        gpi::pc::type::counter_t m_segment_counter;
        segment_map_t m_segments;
        area_map_t m_areas;
        handle_to_segment_t m_handle_to_segment;
      };
    }
  }
}

#endif
