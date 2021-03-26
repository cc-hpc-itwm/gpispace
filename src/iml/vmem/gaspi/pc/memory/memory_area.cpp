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

#include <iml/vmem/gaspi/pc/memory/memory_area.hpp>

#include <iml/vmem/gaspi/pc/global/itopology.hpp>

#include <stack>

#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>

#include <iml/AllocationHandle.hpp>
#include <iml/MemoryOffset.hpp>
#include <iml/MemorySize.hpp>
#include <iml/vmem/gaspi/pc/type/impl_types.hpp>

#include <util-generic/unreachable.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      /***************************************************/
      /*                   area_t                        */
      /***************************************************/

      area_t::area_t (iml::MemorySize size)
        : _local_size (size)
        , m_mmgr (size, 1)
      {
      }

      void area_t::pre_dtor ()
      {
        lock_type lock (m_mutex);

        while (! m_handles.empty())
        {
          gpi::pc::type::handle::descriptor_t d (m_handles.begin()->second);
          try
          {
            this->free ( d.id );
          }
          catch (std::exception const&)
          {
          }
        }
      }

      std::unordered_set<iml::AllocationHandle>
        area_t::existing_allocations() const
      {
        std::unordered_set<iml::AllocationHandle> result;
        for (auto const& kv : m_handles)
        {
          if (kv.second.flags == is_global::yes)
          {
            result.emplace (kv.first);
          }
        }
        return result;
      }

      bool area_t::is_shm_segment() const
      {
        return false;
      }

      bool area_t::is_local (const iml::MemoryRegion region) const
      {
        return is_local (region, region.size);
      }

      bool
      area_t::is_local ( const iml::MemoryLocation location
                       , const iml::MemorySize amount
                       ) const
      {
        lock_type lock (m_mutex);


        handle_descriptor_map_t::const_iterator hdl_it
            (m_handles.find(location.allocation));
        if (hdl_it == m_handles.end())
          throw std::runtime_error ("is_local(): no such handle");

        const iml::MemoryOffset start = location.offset;
        const iml::MemoryOffset end   = start + amount;

        return is_range_local (hdl_it->second, start, end);
      }

      void
      area_t::check_bounds ( const iml::MemoryLocation & loc
                           , const iml::MemorySize size
                           ) const
      {
        lock_type lock (m_mutex);
        handle_descriptor_map_t::const_iterator
            hdl_it (m_handles.find(loc.allocation));
        if (hdl_it == m_handles.end())
          throw std::invalid_argument("check_bounds: no such handle");
        if (! (loc.offset < hdl_it->second.size && (loc.offset + size) <= hdl_it->second.size))
        {
          throw std::invalid_argument
            ( ( boost::format ("out-of-bounds: access to %1%-handle:"
                              " range [%2%,%3%) is not within [0,%4%)"
                              )
              % hdl_it->second.id
              % loc.offset
              % (loc.offset + size)
              % hdl_it->second.size
              ).str()
            );
        }
      }

      namespace
      {
        iml_client::vmem::dtmmgr::Arena_t grow_direction (is_global visibility)
        {
          return visibility == is_global::yes
            ? iml_client::vmem::dtmmgr::ARENA_UP
            : iml_client::vmem::dtmmgr::ARENA_DOWN;
        }
      }

      void
        area_t::remote_alloc ( const iml::AllocationHandle hdl_id
                           , const iml::MemoryOffset offset
                           , const iml::MemorySize size
                           , const iml::MemorySize local_size
                           , iml::SegmentHandle segment_id
                           )
      {
        gpi::pc::type::handle::descriptor_t hdl;
        hdl.id = hdl_id;
        hdl.size = size;
        hdl.local_size = local_size;
        hdl.offset = offset;
        hdl.flags = is_global::yes;

        internal_alloc (hdl, false, segment_id);

        if (hdl.offset != offset)
        {
          m_mmgr.free (hdl.id, grow_direction (hdl.flags));

          throw std::runtime_error
            ( "remote_alloc failed: offset mismatch: expected = "
            + std::to_string (offset) + " actual = "
            + std::to_string (hdl.offset)
            );
        }
        else
        {
          m_handles [hdl.id] = hdl;
        }
      }

      void
      area_t::alloc ( const iml::MemorySize size
                    , const gpi::pc::type::flags_t flags
                    , iml::SegmentHandle segment_id
                    , iml::AllocationHandle allocation
                    )
      {
        lock_type lock (m_mutex);

        gpi::pc::type::handle::descriptor_t hdl;
        hdl.size = size;
        // get distribution scheme
        hdl.local_size = get_local_size (size, flags);
        hdl.flags = flags;
        hdl.id = allocation;

        internal_alloc (hdl, true, segment_id);

        m_handles [hdl.id] = hdl;
      }

      void area_t::internal_alloc ( gpi::pc::type::handle::descriptor_t& hdl
                                  , bool is_creator
                                  , iml::SegmentHandle segment_id
                                  )
      {
        if (m_mmgr.memfree() < hdl.local_size)
        {
          throw std::runtime_error
            ( "out of memory: total size = " + std::to_string (hdl.size)
            + " local size = " + std::to_string (hdl.local_size)
            + " segment = " + segment_id.to_string()
            + " avail = " + std::to_string (m_mmgr.memfree())
            );
        }

        iml_client::vmem::dtmmgr::Arena_t arena = grow_direction(hdl.flags);

        try
        {
          hdl.offset = m_mmgr.alloc (hdl.id, arena, hdl.local_size).first;
        }
        catch (iml_client::vmem::error::alloc::insufficient_contiguous_memory const&)
        {
          throw std::runtime_error
            ( "not enough contiguous memory available: requested_size = "
            + std::to_string (hdl.local_size)
            + " segment = " + segment_id.to_string()
            + " avail = " + std::to_string (m_mmgr.memfree())
            );
        }
        catch (iml_client::vmem::error::alloc::insufficient_memory const&)
        {
          throw std::runtime_error
            ( "not enough memory: requested_size = "
            + std::to_string (hdl.local_size)
            + " segment = " + segment_id.to_string()
            + " avail = " + std::to_string (m_mmgr.memfree())
            );
        }
        catch (iml_client::vmem::error::alloc::duplicate_handle const&)
        {
          throw std::runtime_error
            ( "duplicate handle: handle = " + hdl.id.to_string()
            + " segment " + segment_id.to_string()
            );
        }

        try
        {
          if (hdl.flags == is_global::yes && is_creator)
          {
            global_topology().alloc ( segment_id
                                    , hdl.id
                                    , hdl.offset
                                    , hdl.size
                                    , hdl.local_size
                                    );
          }
        }
        catch (std::exception const&)
        {
          m_mmgr.free (hdl.id, arena);
          std::throw_with_nested (std::runtime_error ("global alloc failed"));
        }
      }

      void area_t::free (const iml::AllocationHandle hdl)
      {
        lock_type const lock (m_mutex);

        auto const desc (descriptor (hdl));

        internal_free (lock, desc);

        try
        {
          if (desc.flags == is_global::yes)
          {
            global_topology().free (desc.id);
          }
        }
        catch (std::exception const&)
        {
          std::throw_with_nested (std::runtime_error ("global free failed"));
        }
      }

      void area_t::remote_free (const iml::AllocationHandle hdl)
      {
        lock_type const lock (m_mutex);

        internal_free (lock, descriptor (hdl));
      }

      void area_t::internal_free
        (lock_type const&, type::handle::descriptor_t const& desc)
      {
        iml_client::vmem::dtmmgr::Arena_t arena (grow_direction(desc.flags));

        m_mmgr.free (desc.id, arena);
        m_handles.erase (desc.id);
      }

      gpi::pc::type::handle::descriptor_t const &
        area_t::descriptor (const iml::AllocationHandle hdl) const
      {
        lock_type lock (m_mutex);
        handle_descriptor_map_t::const_iterator pos (m_handles.find (hdl));
        if (pos != m_handles.end())
        {
          return pos->second;
        }
        else
        {
          throw std::runtime_error
            ("cannot find descriptor for handle " + hdl.to_string());
        }
      }

      iml::MemoryOffset
      area_t::location_to_offset (iml::MemoryLocation loc)
      {
        lock_type lock (m_mutex);

        handle_descriptor_map_t::const_iterator hdl_it
            (m_handles.find(loc.allocation));
        if (hdl_it == m_handles.end())
          throw std::runtime_error
            ( "location_to_offset(): no such handle: "
            + boost::lexical_cast<std::string>(loc.allocation)
            );

        return hdl_it->second.offset + (loc.offset % hdl_it->second.local_size);
      }

      void *
      area_t::pointer_to (iml::MemoryLocation const &loc)
      {
        return raw_ptr (location_to_offset (loc));
      }

      iml::MemorySize
      area_t::read_from ( iml::MemoryLocation loc
                        , void *buffer
                        , iml::MemorySize amount
                        )
      {
        // TODO: set amount to minimum of amount and size-loc.offset
        check_bounds (loc, amount);

        if (is_local (iml::MemoryRegion (loc, amount)))
        {
          return read_from_impl ( location_to_offset (loc)
                                , buffer
                                , amount
                                );
        }
        else
        {
          throw std::runtime_error
            (std::string ("could not read non-local region"));
        }
      }

      iml::MemorySize
      area_t::write_to ( iml::MemoryLocation loc
                       , const void *buffer
                       , iml::MemorySize amount
                       )
      {
        // TODO: set amount to minimum of amount and size-loc.offset
        check_bounds (loc, amount);

        if (is_local (iml::MemoryRegion (loc, amount)))
        {
          return write_to_impl ( location_to_offset (loc)
                               , buffer
                               , amount
                               );
        }
        else
        {
          throw std::runtime_error
            (std::string ("could not read non-local region"));
        }
      }

      iml::MemorySize
      area_t::read_from_impl ( iml::MemoryOffset offset
                             , void *buffer
                             , iml::MemorySize amount
                             )
      {
        std::memmove (buffer, raw_ptr (offset), amount);
        return amount;
      }

      iml::MemorySize
      area_t::write_to_impl ( iml::MemoryOffset offset
                            , const void *buffer
                            , iml::MemorySize amount
                            )
      {
        std::memmove (raw_ptr (offset), buffer, amount);
        return amount;
      }


      std::packaged_task<void()> area_t::get_send_task
        ( area_t&
        , const iml::MemoryLocation
        , const iml::MemoryLocation
        , iml::MemorySize
        )
      {
        throw std::logic_error ("get_send_task not implemented");
      }

      std::packaged_task<void()> area_t::get_recv_task
        ( area_t&
        , const iml::MemoryLocation
        , const iml::MemoryLocation
        , iml::MemorySize
        )
      {
        throw std::logic_error ("get_recv_task not implemented");
      }

      std::packaged_task<void()> area_t::get_transfer_task
        ( const iml::MemoryLocation src
        , const iml::MemoryLocation dst
        , area_t& dst_area
        , iml::MemorySize amount
        )
      {
        const bool src_is_local (is_local (src, amount));
        const bool dst_is_local (dst_area.is_local (dst, amount));

        if (src_is_local && dst_is_local)
        {
          void *src_ptr = pointer_to (src);
          void *dst_ptr = dst_area.pointer_to (dst);

          if (src_ptr)
          {
            return std::packaged_task<void()>
              ( [&dst_area, dst, src_ptr, amount]
                {
                  dst_area.write_to (dst, src_ptr, amount);
                }
              );
          }
          else if (dst_ptr)
          {
            return std::packaged_task<void()>
              ( [this, src, dst_ptr, amount]
                {
                  read_from (src, dst_ptr, amount);
                }
              );
          }
        }

        if (src_is_local)
        {
          return dst_area.get_send_task (*this, src, dst, amount);
        }
        else if (dst_is_local)
        {
          return this->get_recv_task (dst_area, dst, src, amount);
        }
        else
        {
          FHG_UTIL_UNREACHABLE
            ( "one area always has to be local: there is always shm_area "
              "on one side"
            );
        }
      }
    }
  }
}
