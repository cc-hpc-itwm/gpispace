// Copyright (C) 2011-2012,2014-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/MemoryOffset.hpp>
#include <gspc/iml/MemorySize.hpp>
#include <gspc/iml/SegmentDescription.hpp>
#include <gspc/iml/SegmentHandle.hpp>
#include <gspc/iml/SharedMemoryAllocationHandle.hpp>
#include <gspc/iml/vmem/gaspi/pc/memory/handle_generator.hpp>
#include <gspc/iml/vmem/gaspi/pc/memory/memory_area.hpp>
#include <gspc/iml/vmem/gaspi/pc/type/impl_types.hpp>

#include <gspc/util/threadsafe_queue.hpp>

#include <gspc/iml/vmem/gaspi_context.hpp>

#include <boost/thread/scoped_thread.hpp>

#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>


  namespace gpi::pc
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

        manager_t (gspc::iml::vmem::gaspi_context&);
        ~manager_t ();
        manager_t (manager_t const&) = delete;
        manager_t (manager_t&&) = delete;
        manager_t& operator= (manager_t const&) = delete;
        manager_t& operator= (manager_t&&) = delete;

        void clear ();

        std::unordered_set<gspc::iml::SegmentHandle> existing_segments() const;
        std::unordered_set<gspc::iml::AllocationHandle> existing_allocations
          (gspc::iml::SegmentHandle segment) const;

        gspc::iml::SharedMemoryAllocationHandle
          register_shm_segment_and_allocate
            ( gpi::pc::type::process_id_t creator
            , std::shared_ptr<shm_area_t> area
            , gspc::iml::MemorySize size
            );
        void unregister_memory ( gpi::pc::type::process_id_t pid
                               , gspc::iml::SharedMemoryAllocationHandle handle
                               );

        void
        remote_alloc ( gspc::iml::SegmentHandle
                     , gspc::iml::AllocationHandle
                     , gspc::iml::MemoryOffset
                     , gspc::iml::MemorySize size
                     , gspc::iml::MemorySize local_size
                     );

        gspc::iml::AllocationHandle
        alloc ( gspc::iml::SegmentHandle seg_id
              , gspc::iml::MemorySize size
              , gpi::pc::type::flags_t flags
              );

        void remote_free (gspc::iml::AllocationHandle hdl);
        void free (gspc::iml::AllocationHandle hdl);
        gpi::pc::type::handle::descriptor_t info (gspc::iml::AllocationHandle hdl) const;
        std::unordered_map<std::string, double> get_transfer_costs (std::list<gspc::iml::MemoryRegion> const&) const;

        void remove_shm_segments_of (gpi::pc::type::process_id_t);

        type::memcpy_id_t memcpy ( gspc::iml::MemoryLocation const& dst
                                 , gspc::iml::MemoryLocation const& src
                                 , gspc::iml::MemorySize amount
                                 );
        void wait (type::memcpy_id_t const&);

        void
        remote_add_memory ( gspc::iml::SegmentHandle seg_id
                          , gspc::iml::SegmentDescription const& description
                          , unsigned long total_size
                          , global::topology_t& topology
                          );

        gspc::iml::SegmentHandle
        add_memory ( gpi::pc::type::process_id_t proc_id
                   , gspc::iml::SegmentDescription const& description
                   , unsigned long total_size
                   , global::topology_t& topology
                   );

        void
        remote_del_memory ( gspc::iml::SegmentHandle seg_id
                          , global::topology_t& topology
                          );

        // Not called with shm areas, those use unregister_memory().
        void
        del_memory ( gpi::pc::type::process_id_t proc_id
                   , gspc::iml::SegmentHandle seg_id
                   , global::topology_t& topology
                   );

      private:
        using mutex_type = std::recursive_mutex;
        using lock_type = std::unique_lock<mutex_type>;
        using area_map_t = std::unordered_map<gspc::iml::SegmentHandle, area_ptr>;
        using handle_to_segment_t =
          std::unordered_map<gspc::iml::AllocationHandle, gspc::iml::SegmentHandle>;

        void add_area (gspc::iml::SegmentHandle seg_id, area_ptr area);
        area_ptr get_area (gspc::iml::SegmentHandle);
        area_ptr get_area (gspc::iml::SegmentHandle) const;
        area_ptr get_area_by_handle (gspc::iml::AllocationHandle);
        area_ptr get_area_by_handle (gspc::iml::AllocationHandle) const;
        void add_handle ( gspc::iml::AllocationHandle
                        , gspc::iml::SegmentHandle
                        );
        void del_handle (gspc::iml::AllocationHandle);

        mutable mutex_type m_mutex;
        area_map_t m_areas;
        std::map<type::process_id_t, std::set<gspc::iml::SegmentHandle>>
          _shm_segments_by_owner;
        handle_to_segment_t m_handle_to_segment;
        gspc::iml::vmem::gaspi_context& _gaspi_context;

        std::mutex _memcpy_task_guard;
        std::size_t _next_memcpy_id {0};
        gspc::util::interruptible_threadsafe_queue<std::packaged_task<void()>>
          _tasks;
        std::map<std::size_t, std::future<void>> _task_by_id;
        std::vector<std::unique_ptr<::boost::strict_scoped_thread<>>>
          _task_threads;
        decltype (_tasks)::interrupt_on_scope_exit _interrupt_task_queue;

        handle_generator_t _handle_generator;
      };
    }
  }
