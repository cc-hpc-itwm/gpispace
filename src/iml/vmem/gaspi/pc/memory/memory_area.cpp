#include <iml/vmem/gaspi/pc/memory/memory_area.hpp>

#include <iml/vmem/gaspi/pc/global/itopology.hpp>

#include <stack>

#include <iml/util/assert.hpp>

#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>

#include <iml/vmem/gaspi/pc/type/handle.hpp>
#include <iml/vmem/gaspi/pc/type/types.hpp>

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

      area_t::area_t ( const gpi::pc::type::segment::segment_type type
                     , const gpi::pc::type::process_id_t creator
                     , const std::string & name
                     , const gpi::pc::type::size_t size
                     , handle_generator_t& handle_generator
                     )
        : m_descriptor ( IML_GPI_PC_INVAL
                       , type
                       , creator
                       , name
                       , size
                       )
        , m_mmgr (m_descriptor.local_size, 1)
        , _handle_generator (handle_generator)
      {
        reinit ();
      }

      void area_t::reinit ()
      {
        m_mmgr = iml_client::vmem::dtmmgr (m_descriptor.local_size, 1);

        update_descriptor_from_mmgr ();
      }

      void area_t::set_id (const gpi::pc::type::id_t id)
      {
        m_descriptor.id = id;
      }

      gpi::pc::type::id_t area_t::get_id () const
      {
        return m_descriptor.id;
      }

      gpi::pc::type::id_t area_t::get_owner () const
      {
        return m_descriptor.creator;
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
          catch (std::exception const & ex)
          {
          }
        }
      }

      // remove all handles allocated by pid
      void area_t::garbage_collect (const gpi::pc::type::process_id_t pid)
      {
        lock_type lock (m_mutex);
        std::stack<gpi::pc::type::handle_t> garbage_handles;
        for ( handle_descriptor_map_t::const_iterator hdl_it(m_handles.begin())
            ; hdl_it != m_handles.end()
            ; ++hdl_it
            )
        {
          if (hdl_it->second.creator == pid)
          {
            garbage_handles.push (hdl_it->first);
          }
        }

        while (!garbage_handles.empty())
        {
          gpi::pc::type::handle_t hdl (garbage_handles.top ());
          garbage_handles.pop ();
          this->free (hdl);
        }
      }

      std::string const & area_t::name () const
      {
        return m_descriptor.name;
      }

      gpi::pc::type::size_t area_t::size () const
      {
        return m_descriptor.local_size;
      }

      int area_t::type () const
      {
        return m_descriptor.type;
      }

      bool area_t::is_shm_segment() const
      {
        return false;
      }

      bool area_t::is_local (const gpi::pc::type::memory_region_t region) const
      {
        return is_local (region.location, region.size);
      }

      bool
      area_t::is_local ( const gpi::pc::type::memory_location_t location
                       , const gpi::pc::type::size_t amount
                       ) const
      {
        lock_type lock (m_mutex);


        handle_descriptor_map_t::const_iterator hdl_it
            (m_handles.find(location.handle));
        if (hdl_it == m_handles.end())
          throw std::runtime_error ("is_local(): no such handle");

        const gpi::pc::type::offset_t start = location.offset;
        const gpi::pc::type::offset_t end   = start + amount;

        return is_range_local (hdl_it->second, start, end);
      }

      void
      area_t::check_bounds ( const gpi::pc::type::memory_location_t & loc
                           , const gpi::pc::type::size_t size
                           ) const
      {
        lock_type lock (m_mutex);
        handle_descriptor_map_t::const_iterator
            hdl_it (m_handles.find(loc.handle));
        if (hdl_it == m_handles.end())
          throw std::invalid_argument("check_bounds: no such handle");
        if (! (loc.offset < hdl_it->second.size && (loc.offset + size) <= hdl_it->second.size))
        {
          throw std::invalid_argument
            ( ( boost::format ("out-of-bounds: access to %1%-handle %2%:"
                              " range [%3%,%4%) is not withing [0,%5%)"
                              )
              % hdl_it->second.id
              % hdl_it->second.name
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
      area_t::remote_alloc ( const gpi::pc::type::handle_t hdl_id
                           , const gpi::pc::type::offset_t offset
                           , const gpi::pc::type::size_t size
                           , const gpi::pc::type::size_t local_size
                           , const std::string & name
                           )
      {
        gpi::pc::type::handle::descriptor_t hdl;
        hdl.segment = m_descriptor.id;
        hdl.id = hdl_id;
        hdl.size = size;
        hdl.local_size = local_size;
        hdl.name = name;
        hdl.offset = offset;
        hdl.creator = IML_GPI_PC_INVAL;
        hdl.flags = is_global::yes;

        internal_alloc (hdl);

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
          update_descriptor_from_mmgr ();
          m_handles [hdl.id] = hdl;
        }
      }

      gpi::pc::type::handle_t
      area_t::alloc ( const gpi::pc::type::process_id_t proc_id
                    , const gpi::pc::type::size_t size
                    , const std::string & name
                    , const gpi::pc::type::flags_t flags
                    )
      {
        lock_type lock (m_mutex);

        gpi::pc::type::handle::descriptor_t hdl;
        hdl.segment = m_descriptor.id;
        hdl.size = size;
        // get distribution scheme
        hdl.local_size = get_local_size (size, flags);
        hdl.name = name;
        hdl.creator = proc_id;
        hdl.flags = flags;
        hdl.id = _handle_generator.next (m_descriptor.type);

        internal_alloc (hdl);

        update_descriptor_from_mmgr ();
        m_handles [hdl.id] = hdl;

        return hdl.id;
      }

      void area_t::defrag (const gpi::pc::type::size_t)
      {
        throw std::runtime_error ("defrag is not yet implemented");
      }

      void area_t::update_descriptor_from_mmgr()
      {
        m_descriptor.avail = m_mmgr.memfree();
        m_descriptor.allocs =
            m_mmgr.numhandle (iml_client::vmem::dtmmgr::ARENA_UP)
          + m_mmgr.numhandle (iml_client::vmem::dtmmgr::ARENA_DOWN);
        // dtmmgr_numalloc -> total allocs
        // dtmmgr_numfree -> total frees
        m_descriptor.ts.touch();
      }

      void area_t::internal_alloc (gpi::pc::type::handle::descriptor_t &hdl)
      {
        if (m_descriptor.avail < hdl.local_size)
        {
          throw std::runtime_error
            ( "out of memory: total size = " + std::to_string (hdl.size)
            + " local size = " + std::to_string (hdl.local_size)
            + " segment = " + std::to_string (m_descriptor.id)
            + " avail = " + std::to_string (m_descriptor.avail)
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
            + " segment = " + std::to_string (m_descriptor.id)
            + " avail = " + std::to_string (m_descriptor.avail)
            );
        }
        catch (iml_client::vmem::error::alloc::insufficient_memory const&)
        {
          throw std::runtime_error
            ( "not enough memory: requested_size = "
            + std::to_string (hdl.local_size)
            + " segment = " + std::to_string (m_descriptor.id)
            + " avail = " + std::to_string (m_descriptor.avail)
            );
        }
        catch (iml_client::vmem::error::alloc::duplicate_handle const&)
        {
          throw std::runtime_error
            ( "duplicate handle: handle = " + std::to_string (hdl.id)
            + " segment " + std::to_string (m_descriptor.id)
            );
        }

        try
        {
          if (hdl.flags == is_global::yes && hdl.creator != IML_GPI_PC_INVAL)
          {
            global_topology().alloc ( descriptor ().id
                                    , hdl.id
                                    , hdl.offset
                                    , hdl.size
                                    , hdl.local_size
                                    , hdl.name
                                    );
          }
        }
        catch (std::exception const & ex)
        {
          m_mmgr.free (hdl.id, arena);
          std::throw_with_nested (std::runtime_error ("global alloc failed"));
        }
      }

      void area_t::free (const gpi::pc::type::handle_t hdl)
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
        catch (std::exception const & ex)
        {
          std::throw_with_nested (std::runtime_error ("global free failed"));
        }
      }

      void area_t::remote_free (const gpi::pc::type::handle_t hdl)
      {
        lock_type const lock (m_mutex);

        internal_free (lock, descriptor (hdl));
      }

      void area_t::internal_free
        (lock_type const&, type::handle::descriptor_t const& desc)
      {
        if (desc.nref)
        {
          throw std::runtime_error
            ( "handle still in use: handle = " + std::to_string (desc.id)
            + " nref = " + std::to_string (desc.nref)
            );
        }

        iml_client::vmem::dtmmgr::Arena_t arena (grow_direction(desc.flags));

        m_mmgr.free (desc.id, arena);
        m_handles.erase (desc.id);
        update_descriptor_from_mmgr ();
      }

      gpi::pc::type::segment::descriptor_t const &
      area_t::descriptor () const
      {
        lock_type lock (m_mutex);
        return m_descriptor;
      }

      gpi::pc::type::segment::descriptor_t &
      area_t::descriptor ()
      {
        lock_type lock (m_mutex);
        return m_descriptor;
      }

      gpi::pc::type::handle::descriptor_t const &
      area_t::descriptor (const gpi::pc::type::handle_t hdl) const
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
            ("cannot find descriptor for handle " + std::to_string (hdl));
        }
      }

      gpi::pc::type::offset_t
      area_t::location_to_offset (gpi::pc::type::memory_location_t loc)
      {
        lock_type lock (m_mutex);

        handle_descriptor_map_t::const_iterator hdl_it
            (m_handles.find(loc.handle));
        if (hdl_it == m_handles.end())
          throw std::runtime_error
            ( "location_to_offset(): no such handle: "
            + boost::lexical_cast<std::string>(loc.handle)
            );

        return hdl_it->second.offset + (loc.offset % hdl_it->second.local_size);
      }

      void *
      area_t::pointer_to (gpi::pc::type::memory_location_t const &loc)
      {
        return raw_ptr (location_to_offset (loc));
      }

      gpi::pc::type::size_t
      area_t::read_from ( gpi::pc::type::memory_location_t loc
                        , void *buffer
                        , gpi::pc::type::size_t amount
                        )
      {
        // TODO: set amount to minimum of amount and size-loc.offset
        check_bounds (loc, amount);

        if (is_local (gpi::pc::type::memory_region_t (loc, amount)))
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

      gpi::pc::type::size_t
      area_t::write_to ( gpi::pc::type::memory_location_t loc
                       , const void *buffer
                       , gpi::pc::type::size_t amount
                       )
      {
        // TODO: set amount to minimum of amount and size-loc.offset
        check_bounds (loc, amount);

        if (is_local (gpi::pc::type::memory_region_t (loc, amount)))
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

      gpi::pc::type::size_t
      area_t::read_from_impl ( gpi::pc::type::offset_t offset
                             , void *buffer
                             , gpi::pc::type::size_t amount
                             )
      {
        std::memmove (buffer, raw_ptr (offset), amount);
        return amount;
      }

      gpi::pc::type::size_t
      area_t::write_to_impl ( gpi::pc::type::offset_t offset
                            , const void *buffer
                            , gpi::pc::type::size_t amount
                            )
      {
        std::memmove (raw_ptr (offset), buffer, amount);
        return amount;
      }


      std::packaged_task<void()> area_t::get_send_task
        ( area_t&
        , const gpi::pc::type::memory_location_t
        , const gpi::pc::type::memory_location_t
        , gpi::pc::type::size_t
        )
      {
        throw std::logic_error ("get_send_task not implemented");
      }

      std::packaged_task<void()> area_t::get_recv_task
        ( area_t&
        , const gpi::pc::type::memory_location_t
        , const gpi::pc::type::memory_location_t
        , gpi::pc::type::size_t
        )
      {
        throw std::logic_error ("get_recv_task not implemented");
      }

      std::packaged_task<void()> area_t::get_transfer_task
        ( const type::memory_location_t src
        , const type::memory_location_t dst
        , area_t& dst_area
        , type::size_t amount
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
