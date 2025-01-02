// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/vmem/dtmmgr.hpp>

#include <iml/AllocationHandle.hpp>
#include <iml/MemoryLocation.hpp>
#include <iml/MemoryOffset.hpp>
#include <iml/MemoryRegion.hpp>
#include <iml/MemorySize.hpp>
#include <iml/SegmentHandle.hpp>
#include <iml/vmem/gaspi/pc/type/handle_descriptor.hpp>
#include <iml/vmem/gaspi/pc/type/impl_types.hpp>
#include <iml/vmem/gaspi/types.hpp>

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
      class area_t
      {
      public:
        // WORK HERE:
        //    this function *must not* be called from the dtor
        //    otherwise we endup calling pure virtual functions
        void pre_dtor();
        virtual ~area_t () = default;
        area_t (area_t const&) = delete;
        area_t (area_t&&) = delete;
        area_t& operator= (area_t const&) = delete;
        area_t& operator= (area_t&&) = delete;

        /* public interface the basic implementation is the same
           for all kinds of segments.

           specific segments may/must hook into specific implementation details
           though
        */

        virtual bool is_shm_segment() const;
        std::unordered_set<iml::AllocationHandle> existing_allocations() const;

        void
        alloc ( iml::MemorySize size
              , gpi::pc::type::flags_t flags
              , iml::SegmentHandle segment_id
              , iml::AllocationHandle allocation
              );

        void
        remote_alloc ( iml::AllocationHandle hdl
                     , iml::MemoryOffset offset
                     , iml::MemorySize size
                     , iml::MemorySize local_size
                     , iml::SegmentHandle segment_id
                     );

        void
        remote_free (iml::AllocationHandle hdl);

        void
        free (iml::AllocationHandle hdl);

        gpi::pc::type::handle::descriptor_t const&
        descriptor (iml::AllocationHandle) const;

        bool is_local (iml::MemoryRegion region) const;
        bool is_local ( iml::MemoryLocation loc
                      , iml::MemorySize amt
                      ) const;

        void check_bounds ( iml::MemoryLocation const& loc
                          , iml::MemorySize size
                          ) const;

        /**
           Return a raw pointer to the given memory location, if possible.

           It may return nullptr in the following cases:

           - the location is out of bounds
           - the implementation does not support raw pointers.
         */
        void *pointer_to (iml::MemoryLocation const& loc);

        iml::MemorySize
        read_from ( iml::MemoryLocation loc
                  , void *buffer
                  , iml::MemorySize amount
                  );

        iml::MemorySize
        write_to ( iml::MemoryLocation loc
                 , const void *buffer
                 , iml::MemorySize amount
                 );

        std::packaged_task<void()> get_transfer_task
          ( iml::MemoryLocation src
          , iml::MemoryLocation dst
          , area_t& dst_area
          , iml::MemorySize amount
          );
        virtual double get_transfer_costs ( iml::MemoryRegion const&
                                          , gpi::rank_t
                                          ) const = 0;
      protected:
        area_t (iml::MemorySize size);


        iml::MemoryOffset location_to_offset (iml::MemoryLocation loc);

        /* hook functions that need to be overridded by specific segments */

        virtual bool is_range_local ( gpi::pc::type::handle::descriptor_t const&
                                    , iml::MemoryOffset a
                                    , iml::MemoryOffset b
                                    ) const = 0;
        virtual void *raw_ptr (iml::MemoryOffset off) = 0;

        virtual iml::MemorySize get_local_size ( iml::MemorySize size
                                                     , gpi::pc::type::flags_t flags
                                                     ) const = 0;

        virtual std::packaged_task<void()> get_send_task
          ( area_t& src_area
          , iml::MemoryLocation src
          , iml::MemoryLocation dst
          , iml::MemorySize amount
          );

        virtual std::packaged_task<void()> get_recv_task
          ( area_t& dst_area
          , iml::MemoryLocation dst
          , iml::MemoryLocation src
          , iml::MemorySize amount
          );

        virtual iml::MemorySize
        read_from_impl ( iml::MemoryOffset offset
                       , void *buffer
                       , iml::MemorySize amount
                       );

        virtual iml::MemorySize
        write_to_impl ( iml::MemoryOffset offset
                      , const void *buffer
                      , iml::MemorySize amount
                      );

        virtual global::itopology_t& global_topology() = 0;

      private:
        using mutex_type = std::recursive_mutex;
        using lock_type = std::unique_lock<mutex_type>;
        using handle_descriptor_map_t =
          std::unordered_map< iml::AllocationHandle
                            , gpi::pc::type::handle::descriptor_t
                            >;

        void internal_alloc ( gpi::pc::type::handle::descriptor_t&
                            , bool is_creator
                            , iml::SegmentHandle segment_id
                            );
        void internal_free
          (lock_type const&, type::handle::descriptor_t const&);

      private:
        mutable mutex_type m_mutex;
      protected:
        iml::MemorySize const _local_size;
      private:
        iml_client::vmem::dtmmgr m_mmgr;
        handle_descriptor_map_t m_handles;
      };

      using area_ptr_t = std::shared_ptr<area_t>;
    }
  }
}
