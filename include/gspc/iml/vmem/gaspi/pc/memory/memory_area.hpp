// Copyright (C) 2011-2012,2014-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/vmem/dtmmgr.hpp>

#include <gspc/iml/AllocationHandle.hpp>
#include <gspc/iml/MemoryLocation.hpp>
#include <gspc/iml/MemoryOffset.hpp>
#include <gspc/iml/MemoryRegion.hpp>
#include <gspc/iml/MemorySize.hpp>
#include <gspc/iml/SegmentHandle.hpp>
#include <gspc/iml/vmem/gaspi/pc/type/handle_descriptor.hpp>
#include <gspc/iml/vmem/gaspi/pc/type/impl_types.hpp>
#include <gspc/iml/vmem/gaspi/types.hpp>

#include <future>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>


  namespace gpi::pc
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
        std::unordered_set<gspc::iml::AllocationHandle> existing_allocations() const;

        void
        alloc ( gspc::iml::MemorySize size
              , gpi::pc::type::flags_t flags
              , gspc::iml::SegmentHandle segment_id
              , gspc::iml::AllocationHandle allocation
              );

        void
        remote_alloc ( gspc::iml::AllocationHandle hdl
                     , gspc::iml::MemoryOffset offset
                     , gspc::iml::MemorySize size
                     , gspc::iml::MemorySize local_size
                     , gspc::iml::SegmentHandle segment_id
                     );

        void
        remote_free (gspc::iml::AllocationHandle hdl);

        void
        free (gspc::iml::AllocationHandle hdl);

        gpi::pc::type::handle::descriptor_t const&
        descriptor (gspc::iml::AllocationHandle) const;

        bool is_local (gspc::iml::MemoryRegion region) const;
        bool is_local ( gspc::iml::MemoryLocation loc
                      , gspc::iml::MemorySize amt
                      ) const;

        void check_bounds ( gspc::iml::MemoryLocation const& loc
                          , gspc::iml::MemorySize size
                          ) const;

        /**
           Return a raw pointer to the given memory location, if possible.

           It may return nullptr in the following cases:

           - the location is out of bounds
           - the implementation does not support raw pointers.
         */
        void *pointer_to (gspc::iml::MemoryLocation const& loc);

        gspc::iml::MemorySize
        read_from ( gspc::iml::MemoryLocation loc
                  , void *buffer
                  , gspc::iml::MemorySize amount
                  );

        gspc::iml::MemorySize
        write_to ( gspc::iml::MemoryLocation loc
                 , const void *buffer
                 , gspc::iml::MemorySize amount
                 );

        std::packaged_task<void()> get_transfer_task
          ( gspc::iml::MemoryLocation src
          , gspc::iml::MemoryLocation dst
          , area_t& dst_area
          , gspc::iml::MemorySize amount
          );
        virtual double get_transfer_costs ( gspc::iml::MemoryRegion const&
                                          , gpi::rank_t
                                          ) const = 0;
      protected:
        area_t (gspc::iml::MemorySize size);


        gspc::iml::MemoryOffset location_to_offset (gspc::iml::MemoryLocation loc);

        /* hook functions that need to be overridded by specific segments */

        virtual bool is_range_local ( gpi::pc::type::handle::descriptor_t const&
                                    , gspc::iml::MemoryOffset a
                                    , gspc::iml::MemoryOffset b
                                    ) const = 0;
        virtual void *raw_ptr (gspc::iml::MemoryOffset off) = 0;

        virtual gspc::iml::MemorySize get_local_size ( gspc::iml::MemorySize size
                                                     , gpi::pc::type::flags_t flags
                                                     ) const = 0;

        virtual std::packaged_task<void()> get_send_task
          ( area_t& src_area
          , gspc::iml::MemoryLocation src
          , gspc::iml::MemoryLocation dst
          , gspc::iml::MemorySize amount
          );

        virtual std::packaged_task<void()> get_recv_task
          ( area_t& dst_area
          , gspc::iml::MemoryLocation dst
          , gspc::iml::MemoryLocation src
          , gspc::iml::MemorySize amount
          );

        virtual gspc::iml::MemorySize
        read_from_impl ( gspc::iml::MemoryOffset offset
                       , void *buffer
                       , gspc::iml::MemorySize amount
                       );

        virtual gspc::iml::MemorySize
        write_to_impl ( gspc::iml::MemoryOffset offset
                      , const void *buffer
                      , gspc::iml::MemorySize amount
                      );

        virtual global::itopology_t& global_topology() = 0;

      private:
        using mutex_type = std::recursive_mutex;
        using lock_type = std::unique_lock<mutex_type>;
        using handle_descriptor_map_t =
          std::unordered_map< gspc::iml::AllocationHandle
                            , gpi::pc::type::handle::descriptor_t
                            >;

        void internal_alloc ( gpi::pc::type::handle::descriptor_t&
                            , bool is_creator
                            , gspc::iml::SegmentHandle segment_id
                            );
        void internal_free
          (lock_type const&, type::handle::descriptor_t const&);

      private:
        mutable mutex_type m_mutex;
      protected:
        gspc::iml::MemorySize const _local_size;
      private:
        iml_client::vmem::dtmmgr m_mmgr;
        handle_descriptor_map_t m_handles;
      };

      using area_ptr_t = std::shared_ptr<area_t>;
    }
  }
