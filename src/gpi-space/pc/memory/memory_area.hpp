#pragma once

#include <mmgr/dtmmgr.h>

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

#include <fhglog/Logger.hpp>

#include <gpi-space/types.hpp>
#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/handle.hpp>
#include <gpi-space/pc/type/memory_location.hpp>
#include <gpi-space/pc/type/segment_descriptor.hpp>
#include <gpi-space/pc/type/handle_descriptor.hpp>

#include <gpi-space/pc/memory/handle_generator.hpp>
#include <gpi-space/pc/memory/task.hpp>
#include <gpi-space/pc/memory/memory_buffer.hpp>

#include <fhg/util/thread/queue.hpp>

#include <unordered_set>
#include <unordered_map>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class area_t : boost::noncopyable
      {
      public:
        typedef fhg::thread::ptr_queue<buffer_t> memory_pool_t;

        virtual ~area_t ();

        /* public interface the basic implementation is the same
           for all kinds of segments.

           specific segments may/must hook into specific implementation details
           though
        */
        gpi::pc::type::size_t size () const;
        std::string const & name () const;
        bool in_use () const;
        int type () const;
        gpi::pc::type::flags_t flags () const;

        virtual void init ();

        // WORK HERE:
        //    this function *must not* be called from the dtor
        //    otherwise we endup calling pure virtual functions
        void garbage_collect ();
        void garbage_collect (const gpi::pc::type::process_id_t pid);

        void                set_id (const gpi::pc::type::id_t id);
        gpi::pc::type::id_t get_id () const;

        gpi::pc::type::id_t get_owner () const;
        void set_owner (gpi::pc::type::id_t);

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

        void list_allocations ( const gpi::pc::type::process_id_t id
                              , gpi::pc::type::handle::list_t &
                              ) const;

        gpi::pc::type::segment::descriptor_t const &
        descriptor () const;

        gpi::pc::type::segment::descriptor_t & descriptor ();

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

        /**
           Return a raw pointer to the given memory location, if possible.

           It may return nullptr in the following cases:

           - the location is out of bounds
           - the implementation does not support raw pointers.
         */
        void *pointer_to (const gpi::pc::type::memory_location_t & loc);

        gpi::pc::type::size_t
        read_from ( gpi::pc::type::memory_location_t loc
                  , void *buffer
                  , gpi::pc::type::size_t amount
                  );

        gpi::pc::type::size_t
        write_to ( gpi::pc::type::memory_location_t loc
                 , const void *buffer
                 , gpi::pc::type::size_t amount
                 );

        int get_transfer_tasks ( const gpi::pc::type::memory_location_t src
                               , const gpi::pc::type::memory_location_t dst
                               , area_t & dst_area
                               , gpi::pc::type::size_t amount
                               , gpi::pc::type::size_t queue
                               , memory_pool_t & buffer_pool
                               , task_list_t & tasks
                               );
        virtual double get_transfer_costs ( const gpi::pc::type::memory_region_t&
                                          , const gpi::rank_t
                                          ) const = 0;
      protected:
        area_t ( fhg::log::Logger&
               , const gpi::pc::type::segment::segment_type type
               , const gpi::pc::type::process_id_t creator
               , const std::string & name
               , const gpi::pc::type::size_t size
               , const gpi::pc::type::flags_t flags
               , handle_generator_t&
               );

        void reinit ();

        gpi::pc::type::offset_t location_to_offset (gpi::pc::type::memory_location_t loc);

        /* hook functions that need to be overridded by specific segments */
        virtual
        Arena_t grow_direction (const gpi::pc::type::flags_t) const = 0;

        virtual
        bool is_allowed_to_attach (const gpi::pc::type::process_id_t) const;

        virtual bool is_range_local ( const gpi::pc::type::handle::descriptor_t &
                                    , const gpi::pc::type::offset_t a
                                    , const gpi::pc::type::offset_t b
                                    ) const = 0;
        virtual void *raw_ptr (gpi::pc::type::offset_t off) = 0;

        virtual gpi::pc::type::size_t get_local_size ( const gpi::pc::type::size_t size
                                                     , const gpi::pc::type::flags_t flags
                                                     ) const = 0;

        virtual int get_specific_transfer_tasks ( const gpi::pc::type::memory_location_t src
                                                , const gpi::pc::type::memory_location_t dst
                                                , area_t & dst_area
                                                , gpi::pc::type::size_t amount
                                                , gpi::pc::type::size_t queue
                                                , task_list_t & tasks
                                                ) = 0;

        virtual int get_send_tasks ( area_t & src_area
                                   , const gpi::pc::type::memory_location_t src
                                   , const gpi::pc::type::memory_location_t dst
                                   , gpi::pc::type::size_t amount
                                   , gpi::pc::type::size_t queue
                                   , task_list_t & tasks
                                   );

        virtual int get_recv_tasks ( area_t & dst_area
                                   , const gpi::pc::type::memory_location_t dst
                                   , const gpi::pc::type::memory_location_t src
                                   , gpi::pc::type::size_t amount
                                   , gpi::pc::type::size_t queue
                                   , task_list_t & tasks
                                   );

        virtual gpi::pc::type::size_t
        read_from_impl ( gpi::pc::type::offset_t offset
                       , void *buffer
                       , gpi::pc::type::size_t amount
                       );

        virtual gpi::pc::type::size_t
        write_to_impl ( gpi::pc::type::offset_t offset
                      , const void *buffer
                      , gpi::pc::type::size_t amount
                      );

        /*
         hook functions that may be overriden
         */
        virtual void alloc_hook (const gpi::pc::type::handle::descriptor_t &) {}
        virtual void  free_hook (const gpi::pc::type::handle::descriptor_t &) {}
      private:
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
        typedef std::unordered_set <gpi::pc::type::process_id_t> process_ids_t;
        typedef std::unordered_map< gpi::pc::type::handle_t
                                  , gpi::pc::type::handle::descriptor_t
                                  > handle_descriptor_map_t;

        void update_descriptor_from_mmgr ();

        void internal_alloc (gpi::pc::type::handle::descriptor_t &);

      protected:
        fhg::log::Logger& _logger;

      private:
        mutable mutex_type m_mutex;
        gpi::pc::type::segment::descriptor_t m_descriptor;
        DTmmgr_t m_mmgr;
        handle_descriptor_map_t m_handles;
        process_ids_t m_attached_processes;

        handle_generator_t& _handle_generator;
      };

      typedef boost::shared_ptr<area_t> area_ptr_t;
    }
  }
}
