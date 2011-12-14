#ifndef GPI_SPACE_PC_MEMORY_MANAGER_HPP
#define GPI_SPACE_PC_MEMORY_MANAGER_HPP 1

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/signals2.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/counter.hpp>
#include <gpi-space/pc/memory/memory_area.hpp>
#include <gpi-space/pc/memory/transfer_manager.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class manager_t : boost::noncopyable
      {
      public:
        typedef boost::shared_ptr<area_t> area_ptr;

        // signals
        typedef boost::signals2::signal
            <void (const gpi::pc::type::segment_id_t)>
               memory_signal_t;
        typedef boost::signals2::signal
            <void (const gpi::pc::type::handle_t)>
               handle_signal_t;
        typedef boost::signals2::signal
            <void (const gpi::pc::type::segment_id_t,
                   const gpi::pc::type::process_id_t
                  )> process_signal_t;

        memory_signal_t memory_added;
        memory_signal_t memory_removed;

        handle_signal_t handle_allocated;
        handle_signal_t handle_freed;

        process_signal_t process_attached;
        process_signal_t process_detached;

		static manager_t & get()
		{
		  static manager_t m;
		  return m;
        }

        ~manager_t ();

        void clear ();

        gpi::pc::type::segment_id_t
        register_memory( const gpi::pc::type::process_id_t creator
                       , const std::string & name
                       , const gpi::pc::type::size_t size
                       , const gpi::pc::type::flags_t flags
                       );
        void unregister_memory ( const gpi::pc::type::process_id_t pid
                               , const gpi::pc::type::segment_id_t
                               );
        void list_memory (gpi::pc::type::segment::list_t &) const;

        void attach_process ( const gpi::pc::type::process_id_t
                            , const gpi::pc::type::segment_id_t
                            );
        void detach_process ( const gpi::pc::type::process_id_t
                            , const gpi::pc::type::segment_id_t
                            );

        int
        remote_alloc ( const gpi::pc::type::segment_id_t
                     , const gpi::pc::type::handle_t
                     , const gpi::pc::type::offset_t
                     , const gpi::pc::type::size_t
                     , const std::string & name
                     );

        gpi::pc::type::handle_t
        alloc ( const gpi::pc::type::process_id_t proc_id
              , const gpi::pc::type::segment_id_t seg_id
              , const gpi::pc::type::size_t size
              , const std::string & name
              , const gpi::pc::type::flags_t flags
              );

        void remote_free(const gpi::pc::type::handle_t hdl);
        void free (const gpi::pc::type::handle_t hdl);
        gpi::pc::type::handle::descriptor_t info (const gpi::pc::type::handle_t hdl) const;

        void garbage_collect () {}
        void garbage_collect (const gpi::pc::type::process_id_t);
        void list_allocations( const gpi::pc::type::segment_id_t seg
                             , gpi::pc::type::handle::list_t & l
                             ) const;
        void list_allocations(gpi::pc::type::handle::list_t & l) const;

        gpi::pc::type::queue_id_t
        memcpy ( const gpi::pc::type::process_id_t proc_id
               , gpi::pc::type::memory_location_t const & dst
               , gpi::pc::type::memory_location_t const & src
               , const gpi::pc::type::size_t amount
               , const gpi::pc::type::queue_id_t queue
               );

        gpi::pc::type::size_t
        wait_on_queue ( const gpi::pc::type::process_id_t proc_id
                      , const gpi::pc::type::queue_id_t queue
                      );
      private:
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
        typedef boost::unordered_map< gpi::pc::type::segment_id_t
                                    , area_ptr
                                    > area_map_t;
        typedef boost::unordered_map< gpi::pc::type::handle_t
                                    , gpi::pc::type::segment_id_t
                                    > handle_to_segment_t;
        typedef boost::unordered_set<area_ptr> garbage_areas_t;

        manager_t ();

        void add_gpi_memory ();
        area_ptr get_area (const gpi::pc::type::segment_id_t);
        area_ptr get_area (const gpi::pc::type::segment_id_t) const;
        area_ptr get_area_by_handle (const gpi::pc::type::handle_t);
        area_ptr get_area_by_handle (const gpi::pc::type::handle_t) const;
        void add_handle ( const gpi::pc::type::handle_t
                        , const gpi::pc::type::segment_id_t
                        );
        void del_handle (const gpi::pc::type::handle_t);
        void unregister_memory (const gpi::pc::type::segment_id_t);

        void check_boundaries ( gpi::pc::type::memory_location_t const & dst
                              , gpi::pc::type::memory_location_t const & src
                              , const gpi::pc::type::size_t amount
                              ) const;

        mutable mutex_type m_mutex;
        gpi::pc::type::id_t m_ident;
        gpi::pc::type::counter_t m_segment_counter;
        area_map_t m_areas;
        garbage_areas_t m_garbage_areas;
        handle_to_segment_t m_handle_to_segment;
        transfer_manager_t m_transfer_mgr;
      };
    }
  }
}

namespace gpi
{
  namespace pc
  {
  namespace global
  {
	inline
    gpi::pc::memory::manager_t & memory_manager()
    {
	  return gpi::pc::memory::manager_t::get();
    }
  }
  }
}

#endif
