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

#include <iml/vmem/gaspi/pc/memory/manager.hpp>

#include <iml/vmem/gaspi/pc/global/topology.hpp>
#include <iml/vmem/gaspi/pc/memory/beegfs_area.hpp>
#include <iml/vmem/gaspi/pc/memory/gaspi_area.hpp>
#include <iml/vmem/gaspi/pc/memory/memory_area.hpp>
#include <iml/vmem/gaspi/pc/memory/shm_area.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/functor_visitor.hpp>
#include <util-generic/print_exception.hpp>

#include <boost/lexical_cast.hpp>

#include <string>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      namespace
      {
        area_ptr_t create_area ( iml::SegmentDescription const& description
                               , unsigned long total_size
                               , global::topology_t& topology
                               , handle_generator_t& handle_generator
                               , fhg::iml::vmem::gaspi_context& gaspi_context
                               , bool is_creator
                               , iml::SegmentHandle segment_id
                               )
        {
          return fhg::util::visit<area_ptr_t>
            ( description
            , [&] (iml::gaspi::SegmentDescription const& desc)
              {
                return gaspi_area_t::create ( desc
                                            , total_size
                                            , topology
                                            , handle_generator
                                            , gaspi_context
                                            , segment_id
                                            );
              }
            , [&] (iml::beegfs::SegmentDescription const& desc)
              {
                return beegfs_area_t::create
                  (desc, total_size, topology, is_creator);
              }
            );
        }
      }

      manager_t::manager_t (fhg::iml::vmem::gaspi_context& gaspi_context)
        : _gaspi_context (gaspi_context)
        , _next_memcpy_id (0)
        , _interrupt_task_queue (_tasks)
        , _handle_generator (gaspi_context.rank())
      {
        const std::size_t number_of_queues (gaspi_context.number_of_queues());
        for (std::size_t i (0); i < number_of_queues; ++i)
        {
          _task_threads.emplace_back
            ( std::make_unique<boost::strict_scoped_thread<>>
                ( [this]
                  {
                    try
                    {
                      for (;;)
                      {
                        _tasks.get()();
                      }
                    }
                    catch (decltype (_tasks)::interrupted const&)
                    {
                    }
                  }
                )
            );
        }
      }

      manager_t::~manager_t ()
      {
        try
        {
          clear ();
        }
        catch (...)
        { }
      }

      void
      manager_t::clear ()
      {
        // preconditions:
        // make sure that there are no remaining
        // accesses to segments queued

        //     i.e. cancel/remove all items in the memory transfer component
        lock_type lock (m_mutex);
        while (! m_areas.empty())
        {
          auto const area_it (m_areas.begin());
          area_ptr area (area_it->second);

          // WORK HERE:
          //    let this do another thread
          //    and just give him the area_ptr
          area->pre_dtor ();
          m_areas.erase (area_it);
        }
      }

      std::unordered_set<iml::SegmentHandle>
        manager_t::existing_segments() const
      {
        lock_type const lock (m_mutex);
        std::unordered_set<iml::SegmentHandle> result;
        for (auto const& kv : m_areas)
        {
          if (!kv.second->is_shm_segment())
          {
            result.emplace (kv.first);
          }
        }
        return result;
      }
      std::unordered_set<iml::AllocationHandle>
        manager_t::existing_allocations (iml::SegmentHandle segment) const
      {
        lock_type const lock (m_mutex);
        return get_area (segment)->existing_allocations();
      }

      iml::SharedMemoryAllocationHandle
        manager_t::register_shm_segment_and_allocate
          ( gpi::pc::type::process_id_t creator
          , std::shared_ptr<shm_area_t> area
          , iml::MemorySize size
          )
      {
        auto const segment (_handle_generator.next_segment());
        add_area (segment, area);
        _shm_segments_by_owner[creator].emplace (segment);

        auto const allocation (_handle_generator.next_allocation());
        area->alloc (size, is_global::no, segment, allocation);
        add_handle (allocation, segment);

        return iml::SharedMemoryAllocationHandle {allocation};
      }

      void
      manager_t::unregister_memory ( const gpi::pc::type::process_id_t pid
                                   , iml::SharedMemoryAllocationHandle hdl
                                   )
      {
        lock_type lock (m_mutex);

        area_ptr area (get_area_by_handle (hdl));

        auto const mem_id (m_handle_to_segment.at (hdl));

        del_handle (hdl);
        area->free (hdl);

        _shm_segments_by_owner.at (pid).erase (mem_id);

            // WORK HERE:
            //    let this do another thread
            //    and just give him the area_ptr
            area->pre_dtor ();
            m_areas.erase (mem_id);
      }

      void manager_t::remove_shm_segments_of
        (gpi::pc::type::process_id_t const proc_id)
      {
        lock_type lock (m_mutex);

        auto const segments_it (_shm_segments_by_owner.find (proc_id));
        if (segments_it == _shm_segments_by_owner.end())
        {
          return;
        }

        for (auto const& mem_id : segments_it->second)
        {
        auto const area_it (m_areas.find(mem_id));
        auto const area (area_it->second);

            // WORK HERE:
            //    let this do another thread
            //    and just give him the area_ptr
            area->pre_dtor ();
            m_areas.erase (area_it);
        }

        _shm_segments_by_owner.erase (segments_it);
      }

      void manager_t::add_area
        (iml::SegmentHandle seg_id, manager_t::area_ptr area)
      {
        lock_type const lock (m_mutex);

        if (!m_areas.emplace (seg_id, std::move (area)).second)
        {
          throw std::runtime_error
            ("cannot add another gpi segment: id already in use!");
        }
      }

      manager_t::area_ptr
      manager_t::get_area (const iml::SegmentHandle mem_id)
      {
        return static_cast<const manager_t*>(this)->get_area (mem_id);
      }

      manager_t::area_ptr
      manager_t::get_area (const iml::SegmentHandle mem_id) const
      {
        lock_type lock (m_mutex);
        area_map_t::const_iterator area_it (m_areas.find (mem_id));
        if (area_it == m_areas.end())
        {
          throw std::runtime_error ( "no such memory: "
                                   + boost::lexical_cast<std::string>(mem_id)
                                   );
        }
        return area_it->second;
      }

      manager_t::area_ptr
        manager_t::get_area_by_handle (const iml::AllocationHandle hdl)
      {
        return static_cast<const manager_t*>(this)->get_area_by_handle(hdl);
      }

      manager_t::area_ptr
        manager_t::get_area_by_handle (const iml::AllocationHandle hdl) const
      {
        lock_type lock (m_mutex);

        handle_to_segment_t::const_iterator h2s (m_handle_to_segment.find(hdl));
        if (h2s != m_handle_to_segment.end())
        {
          return get_area (h2s->second);
        }
        else
        {
          throw std::runtime_error ( "no such handle: "
                                   + boost::lexical_cast<std::string>(hdl)
                                   );
        }
      }

      void
        manager_t::add_handle ( const iml::AllocationHandle hdl
                            , const iml::SegmentHandle seg_id
                            )
      {
        lock_type lock (m_mutex);
        if (m_handle_to_segment.find (hdl) != m_handle_to_segment.end())
        {
          throw std::runtime_error ("handle does already exist");
        }
        m_handle_to_segment [hdl] = seg_id;
      }

      void
        manager_t::del_handle (const iml::AllocationHandle hdl)
      {
        lock_type lock (m_mutex);
        if (m_handle_to_segment.find (hdl) == m_handle_to_segment.end())
        {
          throw std::runtime_error ("handle does not exist");
        }
        m_handle_to_segment.erase (hdl);
      }

      void
      manager_t::remote_alloc ( const iml::SegmentHandle seg_id
                              , const iml::AllocationHandle hdl
                              , const iml::MemoryOffset offset
                              , const iml::MemorySize size
                              , const iml::MemorySize local_size
                              )
      {
        area_ptr area (get_area (seg_id));
        area->remote_alloc (hdl, offset, size, local_size, seg_id);
        add_handle (hdl, seg_id);
      }

      iml::AllocationHandle
      manager_t::alloc ( const iml::SegmentHandle seg_id
                       , const iml::MemorySize size
                       , const gpi::pc::type::flags_t flags
                       )
      {
        area_ptr area (get_area (seg_id));

        auto const allocation (_handle_generator.next_allocation());
        area->alloc (size, flags, seg_id, allocation);

        add_handle (allocation, seg_id);

        return allocation;
      }

      void
        manager_t::remote_free (const iml::AllocationHandle hdl)
      {
        area_ptr area (get_area_by_handle (hdl));

        area->remote_free (hdl);
        del_handle (hdl);
      }

      void
        manager_t::free (const iml::AllocationHandle hdl)
      {
        area_ptr area (get_area_by_handle (hdl));

        del_handle (hdl);
        area->free (hdl);
      }

      gpi::pc::type::handle::descriptor_t
        manager_t::info (const iml::AllocationHandle hdl) const
      {
        return get_area_by_handle(hdl)->descriptor(hdl);
      }

      std::unordered_map<std::string, double>
      manager_t::get_transfer_costs (const std::list<iml::MemoryRegion>& transfers) const
      {
        std::unordered_map<std::string, double> costs;

        for (auto const& transfer : transfers)
        {
          const area_ptr area (get_area_by_handle (transfer.allocation));

          for (gpi::rank_t rank = 0; rank < _gaspi_context.number_of_nodes(); ++rank)
          {
            costs[_gaspi_context.hostname_of_rank (rank)] += area->get_transfer_costs (transfer, rank);
          }
        }

        return costs;
      }

      type::memcpy_id_t manager_t::memcpy
        ( iml::MemoryLocation const & dst
        , iml::MemoryLocation const & src
        , const iml::MemorySize amount
        )
      {
        auto& src_area (*get_area_by_handle (src.allocation));
        auto& dst_area (*get_area_by_handle (dst.allocation));

        dst_area.check_bounds (dst, amount);
        src_area.check_bounds (src, amount);

        auto task ( src_area.get_transfer_task
                      ( src
                      , dst
                      , dst_area
                      , amount
                      )
                  );

        std::unique_lock<std::mutex> const _ (_memcpy_task_guard);
        type::memcpy_id_t const memcpy_id (_next_memcpy_id++);

        _task_by_id.emplace (memcpy_id, task.get_future());
        _tasks.put (std::move (task));

        return memcpy_id;
      }

      void manager_t::wait (type::memcpy_id_t const& memcpy_id)
      {
        decltype (_task_by_id)::iterator task_it (_task_by_id.end());

        {
          std::unique_lock<std::mutex> const _ (_memcpy_task_guard);

          task_it = _task_by_id.find (memcpy_id);
          if (task_it == _task_by_id.end())
          {
            throw std::invalid_argument ("no such memcpy id");
          }
        }

        FHG_UTIL_FINALLY
          ( [&]
            {
              std::unique_lock<std::mutex> const _ (_memcpy_task_guard);
              _task_by_id.erase (task_it);
            }
          );

        task_it->second.get();
      }

      void
      manager_t::remote_add_memory ( const iml::SegmentHandle seg_id
                                   , iml::SegmentDescription const& description
                                   , unsigned long total_size
                                   , global::topology_t& topology
                                   )
      {
        area_ptr_t area (create_area (description, total_size, topology, _handle_generator, _gaspi_context, false, seg_id));
        add_area (seg_id, std::move (area));
      }

      namespace
      {
        //! \note for beegfs, master creates file that slaves need to
        //! open determining filesize from the opened file. thus, to
        //! avoid a race, the master is doing this before all
        //! others. gaspi on the other side needs simultaneous
        //! initialization for gaspi_segment_create etc.
        bool require_earlier_master_initialization
          (iml::SegmentDescription const& description)
        {
          return fhg::util::visit<bool>
            ( description
            , [&] (iml::beegfs::SegmentDescription const&)
              {
                return true;
              }
            , [&] (iml::gaspi::SegmentDescription const&)
              {
                return false;
              }
            );
        }
      }

      iml::SegmentHandle
      manager_t::add_memory ( const gpi::pc::type::process_id_t proc_id
                            , iml::SegmentDescription const& description
                            , unsigned long total_size
                            , global::topology_t& topology
                            )
      {
        auto const id (_handle_generator.next_segment());
        bool successfully_added (false);
        FHG_UTIL_FINALLY ( [&]
                           {
                             if (!successfully_added)
                             {
                               try
                               {
                                 del_memory (proc_id, id, topology);
                               }
                               catch (...)
                               {
                                 //! \note avoid abort
                               }
                             }
                           }
                         );

        if (require_earlier_master_initialization (description))
        {
          area_ptr_t area = create_area (description, total_size, topology, _handle_generator, _gaspi_context, true, id);
          add_area (id, std::move (area));

          topology.add_memory (id, description, total_size);
        }
        else
        {
          std::future<void> local
            ( std::async
                ( std::launch::async
                , [&]
                  {
                    area_ptr_t area ( create_area ( description
                                                  , total_size
                                                  , topology
                                                  , _handle_generator
                                                  , _gaspi_context
                                                  , true
                                                  , id
                                                  )
                                    );
                    add_area (id, std::move (area));
                  }
                )
            );

          topology.add_memory (id, description, total_size);
          local.get();
        }

        successfully_added = true;
        return id;
      }

      void
      manager_t::remote_del_memory ( const iml::SegmentHandle seg_id
                                   , global::topology_t& topology
                                   )
      {
        del_memory (0, seg_id, topology);
      }

      void
      manager_t::del_memory ( const gpi::pc::type::process_id_t proc_id
                            , const iml::SegmentHandle seg_id
                            , global::topology_t& topology
                            )
      {
          lock_type lock (m_mutex);

          area_map_t::iterator area_it (m_areas.find (seg_id));
          if (area_it == m_areas.end ())
          {
            throw std::runtime_error ( "no such memory: "
                                     + boost::lexical_cast<std::string>(seg_id)
                                     );
          }

          area_ptr area (area_it->second);

          area->pre_dtor ();
          m_areas.erase (area_it);

          if (proc_id > 0)
          {
            topology.del_memory (seg_id);
          }
      }
    }
  }
}
