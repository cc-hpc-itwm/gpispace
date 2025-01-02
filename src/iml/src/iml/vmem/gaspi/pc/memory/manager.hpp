// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/MemoryOffset.hpp>
#include <iml/MemorySize.hpp>
#include <iml/SegmentDescription.hpp>
#include <iml/SegmentHandle.hpp>
#include <iml/SharedMemoryAllocationHandle.hpp>
#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>
#include <iml/vmem/gaspi/pc/memory/memory_area.hpp>
#include <iml/vmem/gaspi/pc/type/impl_types.hpp>

#include <util-generic/threadsafe_queue.hpp>

#include <iml/vmem/gaspi_context.hpp>

#include <boost/thread/scoped_thread.hpp>

#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      class topology_t;
    }
    namespace memory
    {
      class shm_area_t;

      class manager_t
      {
      public:
        using area_ptr = area_ptr_t;

        manager_t (fhg::iml::vmem::gaspi_context&);
        ~manager_t ();
        manager_t (manager_t const&) = delete;
        manager_t (manager_t&&) = delete;
        manager_t& operator= (manager_t const&) = delete;
        manager_t& operator= (manager_t&&) = delete;

        void clear ();

        std::unordered_set<iml::SegmentHandle> existing_segments() const;
        std::unordered_set<iml::AllocationHandle> existing_allocations
          (iml::SegmentHandle segment) const;

        iml::SharedMemoryAllocationHandle
          register_shm_segment_and_allocate
            ( gpi::pc::type::process_id_t creator
            , std::shared_ptr<shm_area_t> area
            , iml::MemorySize size
            );
        void unregister_memory ( gpi::pc::type::process_id_t pid
                               , iml::SharedMemoryAllocationHandle handle
                               );

        void
        remote_alloc ( iml::SegmentHandle
                     , iml::AllocationHandle
                     , iml::MemoryOffset
                     , iml::MemorySize size
                     , iml::MemorySize local_size
                     );

        iml::AllocationHandle
        alloc ( iml::SegmentHandle seg_id
              , iml::MemorySize size
              , gpi::pc::type::flags_t flags
              );

        void remote_free (iml::AllocationHandle hdl);
        void free (iml::AllocationHandle hdl);
        gpi::pc::type::handle::descriptor_t info (iml::AllocationHandle hdl) const;
        std::unordered_map<std::string, double> get_transfer_costs (std::list<iml::MemoryRegion> const&) const;

        void remove_shm_segments_of (gpi::pc::type::process_id_t);

        type::memcpy_id_t memcpy ( iml::MemoryLocation const& dst
                                 , iml::MemoryLocation const& src
                                 , iml::MemorySize amount
                                 );
        void wait (type::memcpy_id_t const&);

        void
        remote_add_memory ( iml::SegmentHandle seg_id
                          , iml::SegmentDescription const& description
                          , unsigned long total_size
                          , global::topology_t& topology
                          );

        iml::SegmentHandle
        add_memory ( gpi::pc::type::process_id_t proc_id
                   , iml::SegmentDescription const& description
                   , unsigned long total_size
                   , global::topology_t& topology
                   );

        void
        remote_del_memory ( iml::SegmentHandle seg_id
                          , global::topology_t& topology
                          );

        // Not called with shm areas, those use unregister_memory().
        void
        del_memory ( gpi::pc::type::process_id_t proc_id
                   , iml::SegmentHandle seg_id
                   , global::topology_t& topology
                   );

      private:
        using mutex_type = std::recursive_mutex;
        using lock_type = std::unique_lock<mutex_type>;
        using area_map_t = std::unordered_map<iml::SegmentHandle, area_ptr>;
        using handle_to_segment_t =
          std::unordered_map<iml::AllocationHandle, iml::SegmentHandle>;

        void add_area (iml::SegmentHandle seg_id, area_ptr area);
        area_ptr get_area (iml::SegmentHandle);
        area_ptr get_area (iml::SegmentHandle) const;
        area_ptr get_area_by_handle (iml::AllocationHandle);
        area_ptr get_area_by_handle (iml::AllocationHandle) const;
        void add_handle ( iml::AllocationHandle
                        , iml::SegmentHandle
                        );
        void del_handle (iml::AllocationHandle);

        mutable mutex_type m_mutex;
        area_map_t m_areas;
        std::map<type::process_id_t, std::set<iml::SegmentHandle>>
          _shm_segments_by_owner;
        handle_to_segment_t m_handle_to_segment;
        fhg::iml::vmem::gaspi_context& _gaspi_context;

        std::mutex _memcpy_task_guard;
        std::size_t _next_memcpy_id {0};
        fhg::util::interruptible_threadsafe_queue<std::packaged_task<void()>>
          _tasks;
        std::map<std::size_t, std::future<void>> _task_by_id;
        std::vector<std::unique_ptr<::boost::strict_scoped_thread<>>>
          _task_threads;
        decltype (_tasks)::interrupt_on_scope_exit _interrupt_task_queue;

        handle_generator_t _handle_generator;
      };
    }
  }
}
