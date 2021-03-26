// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <boost/noncopyable.hpp>

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

      class manager_t : boost::noncopyable
      {
      public:
        using area_ptr = area_ptr_t;

        manager_t (fhg::iml::vmem::gaspi_context&);
        ~manager_t ();

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
        void unregister_memory ( const gpi::pc::type::process_id_t pid
                               , iml::SharedMemoryAllocationHandle handle
                               );

        void
        remote_alloc ( const iml::SegmentHandle
                     , const iml::AllocationHandle
                     , const iml::MemoryOffset
                     , const iml::MemorySize size
                     , const iml::MemorySize local_size
                     );

        iml::AllocationHandle
        alloc ( const iml::SegmentHandle seg_id
              , const iml::MemorySize size
              , const gpi::pc::type::flags_t flags
              );

        void remote_free(const iml::AllocationHandle hdl);
        void free (const iml::AllocationHandle hdl);
        gpi::pc::type::handle::descriptor_t info (const iml::AllocationHandle hdl) const;
        std::unordered_map<std::string, double> get_transfer_costs (const std::list<iml::MemoryRegion>&) const;

        void remove_shm_segments_of (gpi::pc::type::process_id_t);

        type::memcpy_id_t memcpy ( iml::MemoryLocation const & dst
                                 , iml::MemoryLocation const & src
                                 , const iml::MemorySize amount
                                 );
        void wait (type::memcpy_id_t const&);

        void
        remote_add_memory ( const iml::SegmentHandle seg_id
                          , iml::SegmentDescription const& description
                          , unsigned long total_size
                          , global::topology_t& topology
                          );

        iml::SegmentHandle
        add_memory ( const gpi::pc::type::process_id_t proc_id
                   , iml::SegmentDescription const& description
                   , unsigned long total_size
                   , global::topology_t& topology
                   );

        void
        remote_del_memory ( const iml::SegmentHandle seg_id
                          , global::topology_t& topology
                          );

        // Not called with shm areas, those use unregister_memory().
        void
        del_memory ( const gpi::pc::type::process_id_t proc_id
                   , const iml::SegmentHandle seg_id
                   , global::topology_t& topology
                   );

      private:
        typedef std::recursive_mutex mutex_type;
        typedef std::unique_lock<mutex_type> lock_type;
        typedef std::unordered_map< iml::SegmentHandle
                                  , area_ptr
                                  > area_map_t;
        typedef std::unordered_map< iml::AllocationHandle
                                  , iml::SegmentHandle
                                  > handle_to_segment_t;

        void add_area (iml::SegmentHandle seg_id, area_ptr area);
        area_ptr get_area (const iml::SegmentHandle);
        area_ptr get_area (const iml::SegmentHandle) const;
        area_ptr get_area_by_handle (const iml::AllocationHandle);
        area_ptr get_area_by_handle (const iml::AllocationHandle) const;
        void add_handle ( const iml::AllocationHandle
                        , const iml::SegmentHandle
                        );
        void del_handle (const iml::AllocationHandle);

        mutable mutex_type m_mutex;
        area_map_t m_areas;
        std::map<type::process_id_t, std::set<iml::SegmentHandle>>
          _shm_segments_by_owner;
        handle_to_segment_t m_handle_to_segment;
        fhg::iml::vmem::gaspi_context& _gaspi_context;

        std::mutex _memcpy_task_guard;
        std::size_t _next_memcpy_id;
        fhg::util::interruptible_threadsafe_queue<std::packaged_task<void()>>
          _tasks;
        std::map<std::size_t, std::future<void>> _task_by_id;
        std::vector<std::unique_ptr<boost::strict_scoped_thread<>>>
          _task_threads;
        decltype (_tasks)::interrupt_on_scope_exit _interrupt_task_queue;

        handle_generator_t _handle_generator;
      };
    }
  }
}
