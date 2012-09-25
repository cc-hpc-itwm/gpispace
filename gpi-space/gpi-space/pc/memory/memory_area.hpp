#ifndef GPI_SPACE_PC_MEMORY_MEMORY_AREA_HPP
#define GPI_SPACE_PC_MEMORY_MEMORY_AREA_HPP 1

#include <mmgr/dtmmgr.h>

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/signals2.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/handle.hpp>
#include <gpi-space/pc/type/memory_location.hpp>
#include <gpi-space/pc/type/segment_descriptor.hpp>
#include <gpi-space/pc/type/handle_descriptor.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class area_t : boost::noncopyable
      {
      public:
        virtual ~area_t ();

        enum grow_direction_t
        {
          GROW_UP
        , GROW_DOWN
        };

        /* public interface the basic implementation is the same
           for all kinds of segments.

           specific segments may/must hook into specific implementation details
           though
        */
        gpi::pc::type::size_t size () const;
        std::string const & name () const;
        bool in_use () const;
        int type () const;

        // WORK HERE:
        //    this function *must not* be called from the dtor
        //    otherwise we endup calling pure virtual functions
        void garbage_collect ();
        void garbage_collect (const gpi::pc::type::process_id_t pid);

        gpi::pc::type::handle_t
        alloc ( const gpi::pc::type::process_id_t proc_id
              , const gpi::pc::type::size_t size
              , const std::string & name
              , const gpi::pc::type::flags_t flags
              );

        int
        remote_alloc ( const gpi::pc::type::handle_t hdl
                     , const gpi::pc::type::offset_t offset
                     , const gpi::pc::type::size_t size
                     , const gpi::pc::type::size_t local_size
                     , const std::string & name
                     );

        void
        remote_free (const gpi::pc::type::handle_t hdl);

        void
        free (const gpi::pc::type::handle_t hdl);

        void defrag (const gpi::pc::type::size_t free_at_least = 0);

        void list_allocations (gpi::pc::type::handle::list_t &) const;

        gpi::pc::type::segment::descriptor_t const &
        descriptor () const;

        gpi::pc::type::handle::descriptor_t const &
        descriptor (const gpi::pc::type::handle_t) const;

        gpi::pc::type::size_t
        attach_process (const gpi::pc::type::process_id_t);

        gpi::pc::type::size_t
        detach_process (const gpi::pc::type::process_id_t);

        bool is_local (const gpi::pc::type::memory_region_t region) const;
        bool is_local ( const gpi::pc::type::memory_location_t loc
                      , const gpi::pc::type::size_t amt
                      ) const;
        bool is_eligible_for_deletion () const;

        bool is_process_attached (const gpi::pc::type::process_id_t) const;

        void check_bounds ( const gpi::pc::type::memory_location_t & loc
                          , const gpi::pc::type::size_t size
                          ) const;

        void *pointer_to (const gpi::pc::type::memory_location_t & loc);
      protected:
        area_t ( const gpi::pc::type::segment::segment_type type
               , const gpi::pc::type::id_t id
               , const gpi::pc::type::process_id_t creator
               , const std::string & name
               , const gpi::pc::type::size_t size
               , const gpi::pc::type::flags_t flags
               );

        /* hook functions that need to be overridded by specific segments */
        virtual
        grow_direction_t grow_direction (const gpi::pc::type::flags_t) const = 0;

        virtual
        bool is_allowed_to_attach (const gpi::pc::type::process_id_t) const = 0;

        virtual void check_bounds ( const gpi::pc::type::handle::descriptor_t &
                                  , const gpi::pc::type::offset_t start
                                  , const gpi::pc::type::offset_t end
                                  ) const = 0;

        virtual bool is_range_local ( const gpi::pc::type::handle::descriptor_t &
                                    , const gpi::pc::type::offset_t a
                                    , const gpi::pc::type::offset_t b
                                    ) const = 0;
        virtual void *ptr() = 0;

        virtual gpi::pc::type::size_t get_local_size ( const gpi::pc::type::size_t size
                                                     , const gpi::pc::type::flags_t flags
                                                     ) const = 0;

        /*
         hook functions that may be overriden
         */
        virtual void alloc_hook (const gpi::pc::type::handle::descriptor_t &) {}
        virtual void  free_hook (const gpi::pc::type::handle::descriptor_t &) {}
      private:
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
        typedef boost::unordered_set <gpi::pc::type::process_id_t> process_ids_t;
        typedef boost::unordered_map< gpi::pc::type::handle_t
                                    , gpi::pc::type::handle::descriptor_t
                                    > handle_descriptor_map_t;

        void update_descriptor_from_mmgr ();

        void internal_alloc (gpi::pc::type::handle::descriptor_t &);

        mutable mutex_type m_mutex;
        gpi::pc::type::segment::descriptor_t m_descriptor;
        DTmmgr_t m_mmgr;
        handle_descriptor_map_t m_handles;
        process_ids_t m_attached_processes;
      };
    }
  }
}

#endif
