#pragma once

#include <iml/vmem/dtmmgr.hpp>

#include <boost/noncopyable.hpp>

#include <iml/vmem/gaspi/types.hpp>
#include <iml/vmem/gaspi/pc/type/types.hpp>
#include <iml/vmem/gaspi/pc/type/handle.hpp>
#include <iml/vmem/gaspi/pc/type/memory_location.hpp>
#include <iml/vmem/gaspi/pc/type/handle_descriptor.hpp>

#include <future>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      class itopology_t;
    }
    namespace memory
    {
      class area_t : boost::noncopyable
      {
      public:
        // WORK HERE:
        //    this function *must not* be called from the dtor
        //    otherwise we endup calling pure virtual functions
        void pre_dtor();
        virtual ~area_t () = default;

        /* public interface the basic implementation is the same
           for all kinds of segments.

           specific segments may/must hook into specific implementation details
           though
        */

        virtual bool is_shm_segment() const;
        std::unordered_set<type::handle_id_t> existing_allocations() const;

        void
        alloc ( const gpi::pc::type::size_t size
              , const std::string & name
              , const gpi::pc::type::flags_t flags
              , type::segment_id_t segment_id
              , type::handle_t allocation
              );

        void
        remote_alloc ( const gpi::pc::type::handle_t hdl
                     , const gpi::pc::type::offset_t offset
                     , const gpi::pc::type::size_t size
                     , const gpi::pc::type::size_t local_size
                     , const std::string & name
                     , type::segment_id_t segment_id
                     );

        void
        remote_free (const gpi::pc::type::handle_t hdl);

        void
        free (const gpi::pc::type::handle_t hdl);

        gpi::pc::type::handle::descriptor_t const &
        descriptor (const gpi::pc::type::handle_t) const;

        bool is_local (const gpi::pc::type::memory_region_t region) const;
        bool is_local ( const gpi::pc::type::memory_location_t loc
                      , const gpi::pc::type::size_t amt
                      ) const;

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

        std::packaged_task<void()> get_transfer_task
          ( const type::memory_location_t src
          , const type::memory_location_t dst
          , area_t& dst_area
          , type::size_t amount
          );
        virtual double get_transfer_costs ( const gpi::pc::type::memory_region_t&
                                          , const gpi::rank_t
                                          ) const = 0;
      protected:
        area_t (gpi::pc::type::size_t size);


        gpi::pc::type::offset_t location_to_offset (gpi::pc::type::memory_location_t loc);

        /* hook functions that need to be overridded by specific segments */

        virtual bool is_range_local ( const gpi::pc::type::handle::descriptor_t &
                                    , const gpi::pc::type::offset_t a
                                    , const gpi::pc::type::offset_t b
                                    ) const = 0;
        virtual void *raw_ptr (gpi::pc::type::offset_t off) = 0;

        virtual gpi::pc::type::size_t get_local_size ( const gpi::pc::type::size_t size
                                                     , const gpi::pc::type::flags_t flags
                                                     ) const = 0;

        virtual std::packaged_task<void()> get_send_task
          ( area_t& src_area
          , const gpi::pc::type::memory_location_t src
          , const gpi::pc::type::memory_location_t dst
          , gpi::pc::type::size_t amount
          );

        virtual std::packaged_task<void()> get_recv_task
          ( area_t& dst_area
          , const gpi::pc::type::memory_location_t dst
          , const gpi::pc::type::memory_location_t src
          , gpi::pc::type::size_t amount
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

        virtual global::itopology_t& global_topology() = 0;

      private:
        typedef std::recursive_mutex mutex_type;
        typedef std::unique_lock<mutex_type> lock_type;
        typedef std::unordered_map< gpi::pc::type::handle_t
                                  , gpi::pc::type::handle::descriptor_t
                                  > handle_descriptor_map_t;

        void internal_alloc ( gpi::pc::type::handle::descriptor_t&
                            , bool is_creator
                            , type::segment_id_t segment_id
                            );
        void internal_free
          (lock_type const&, type::handle::descriptor_t const&);

      private:
        mutable mutex_type m_mutex;
      protected:
        type::size_t const _local_size;
      private:
        iml_client::vmem::dtmmgr m_mmgr;
        handle_descriptor_map_t m_handles;
      };

      typedef std::shared_ptr<area_t> area_ptr_t;
    }
  }
}
