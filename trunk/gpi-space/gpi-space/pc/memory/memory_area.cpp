#include "memory_area.hpp"

#include <stack>

#include <fhglog/minimal.hpp>

#include <gpi-space/pc/type/handle.hpp>
#include <gpi-space/pc/memory/handle_generator.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      static Arena_t translate_grow_direction (int dir)
      {
        switch (dir)
        {
        case area_t::GROW_UP:
          return ARENA_UP;
          break;
        case area_t::GROW_DOWN:
          return ARENA_DOWN;
        default:
          throw std::runtime_error ("invalid grow direction");
        }
      }

      /***************************************************/
      /*                   area_t                        */
      /***************************************************/

      area_t::area_t ( const gpi::pc::type::segment::segment_type type
                     , const gpi::pc::type::id_t id
                     , const gpi::pc::type::process_id_t creator
                     , const std::string & name
                     , const gpi::pc::type::size_t size
                     , const gpi::pc::type::flags_t flags
                     )
          : m_descriptor ( id
                         , type
                         , creator
                         , name
                         , size
                         , flags
                         )
          , m_mmgr (NULL)
      {
        dtmmgr_init (&m_mmgr, size, 1);
      }

      area_t::~area_t ()
      {
        LOG_IF ( WARN
               , m_handles.size()
               , "there are still handles left at destruction time."
               << " area = " << m_descriptor.id
               << " handles = " << m_handles.size()
               );
        dtmmgr_finalize (&m_mmgr);
      }

      void area_t::garbage_collect ()
      {
        lock_type lock (m_mutex);

        while (! m_handles.empty())
        {
          gpi::pc::type::handle::descriptor_t d (m_handles.begin()->second);
          try
          {
            this->free ( d.id );
            LOG(DEBUG, "cleaned up possibly leaked handle: " << d);
          }
          catch (std::exception const & ex)
          {
            LOG(WARN, "could not free handle: " << d);
          }
        }
      }

      // remove all non persistent handles allocated by pid
      void area_t::garbage_collect (const gpi::pc::type::process_id_t pid)
      {
        lock_type lock (m_mutex);
        std::stack<gpi::pc::type::handle_t> garbage_handles;
        for ( handle_descriptor_map_t::const_iterator hdl_it(m_handles.begin())
            ; hdl_it != m_handles.end()
            ; ++hdl_it
            )
        {
          if ( hdl_it->second.creator == pid
             and not gpi::flag::is_set ( hdl_it->second.flags
                                       , gpi::pc::type::handle::F_PERSISTENT
                                       )
             )
          {
            garbage_handles.push (hdl_it->first);
          }
        }

        while (!garbage_handles.empty())
        {
          gpi::pc::type::handle_t hdl (garbage_handles.top ());
          garbage_handles.pop ();
          LOG(DEBUG, "garbage collecting handle " << hdl << " allocated by process " << pid);
          this->free (hdl);
        }
      }

      bool area_t::in_use () const
      {
        lock_type lock (m_mutex);
        return m_descriptor.nref > 0;
      }

      gpi::pc::type::size_t area_t::size () const
      {
        return m_descriptor.size;
      }

      int area_t::type () const
      {
        return m_descriptor.type;
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
        if (0 == amount)
          throw std::runtime_error ("is_local(): empty region");

        const gpi::pc::type::offset_t start
          (location.offset);
        const gpi::pc::type::offset_t end
          (start + (amount - 1));
        return is_range_local (hdl_it->second, start, end);
      }

      bool area_t::is_eligible_for_deletion () const
      {
        lock_type lock (m_mutex);
        assert (m_descriptor.nref == m_attached_processes.size());
        if (m_descriptor.nref)
        {
          return false;
        }
        else if (gpi::flag::is_set ( m_descriptor.flags
                                   , gpi::pc::type::segment::F_PERSISTENT
                                   ))
        {
          return false;
        }
        else
        {
          return true;
        }
      }

      bool
      area_t::is_process_attached (const gpi::pc::type::process_id_t proc_id) const
      {
        lock_type lock (m_mutex);
        return m_attached_processes.find (proc_id) != m_attached_processes.end();
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
        check_bounds (hdl_it->second, loc.offset, loc.offset+size-1);
      }

      int
      area_t::remote_alloc ( const gpi::pc::type::handle_t hdl_id
                           , const gpi::pc::type::offset_t offset
                           , const gpi::pc::type::size_t size
                           , const std::string & name
                           )
      {
        gpi::pc::type::handle::descriptor_t hdl;
        hdl.segment = m_descriptor.id;
        hdl.id = hdl_id;
        hdl.size = size;
        hdl.name = name;
        hdl.offset = offset;
        hdl.creator = (gpi::pc::type::process_id_t)(-1);
        hdl.flags = type::handle::F_GLOBAL | type::handle::F_PERSISTENT;

        Arena_t arena = translate_grow_direction(grow_direction(hdl.flags));

        AllocReturn_t alloc_return
            (dtmmgr_alloc (&m_mmgr, hdl_id, arena, size));

        DLOG( TRACE
            , "ALLOC:"
            << " handle = " << hdl.id
            << " arena = " << arena
            << " size = " << size
            << " return = " << alloc_return
            );

        switch (alloc_return)
        {
        case ALLOC_SUCCESS:
          {
            Offset_t actual_offset (0);
            dtmmgr_offset_size ( m_mmgr
                               , hdl.id
                               , arena
                               , &actual_offset
                               , NULL
                               );
            if (actual_offset != offset)
            {
              LOG(ERROR, "remote_alloc failed: expected-offset = " << offset << " actual-offset = " << actual_offset);
              dtmmgr_free (&m_mmgr, hdl.id, arena);
              throw std::runtime_error("offset mismatch");
            }
            else
            {
              update_descriptor_from_mmgr ();
              m_handles [hdl.id] = hdl;
            }
          }
          break;
        case ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY:
          LOG( WARN
             , "not enough contiguous memory available:"
             << " requested_size = " << size
             << " segment = " << m_descriptor.id
             << " avail = " << m_descriptor.avail
             );
          // TODO:
          //    defrag (size);
          //        release locks (? how)
          //          block all accesses to this area
          //              // memcpy/allocs should return EAGAIN
          //          wait for transactions to finish
          //          real_defrag
          //        reacquire locks
          throw std::runtime_error
              ("not enough contiguous memory, defrag not yet implemented");
          break;
        case ALLOC_INSUFFICIENT_MEMORY:
          LOG( ERROR
             , "not enough memory:"
             << " requested_size=" << size
             << " segment=" << m_descriptor.id
             << " avail=" << m_descriptor.avail
             );
          throw std::runtime_error ("out of memory");
          break;
        case ALLOC_DUPLICATE_HANDLE:
          LOG( ERROR
             ,  "duplicate handle:"
             << " handle = " << hdl.id
             << " segment " << m_descriptor.id
             );
          throw std::runtime_error ("duplicate handle");
          break;
        case ALLOC_FAILURE:
          LOG( ERROR
             , "internal error during allocation:"
             << " requested_size = " << size
             << " handle = " << hdl.id
             << " segment = " << m_descriptor.id
             );
          throw std::runtime_error ("allocation failed");
          break;
        default:
          LOG( ERROR
             ,  "unexpected error during allocation:"
             << " requested_size = " << size
             << " handle = " << hdl.id
             << " segment = " << m_descriptor.id
             << " error = " << alloc_return
             );
          throw std::runtime_error ("unexpected return code");
          break;
        }

        return 0;
      }

      gpi::pc::type::handle_t
      area_t::alloc ( const gpi::pc::type::process_id_t proc_id
                    , const gpi::pc::type::size_t size
                    , const std::string & name
                    , const gpi::pc::type::flags_t flags
                    )
      {
        lock_type lock (m_mutex);

        // avoid generation of handle if alloc would definitely fail
        if (m_descriptor.avail < size)
        {
          LOG( ERROR
             , "not enough memory:"
             << " requested_size = " << size
             << " segment = " << m_descriptor.id
             << " avail = " << m_descriptor.avail
             );
          throw std::runtime_error ("out of memory");
        }

        gpi::pc::type::handle::descriptor_t hdl;
        hdl.segment = m_descriptor.id;
        hdl.id =
            handle_generator_t::get().next(m_descriptor.type);
        hdl.size = size;
        hdl.name = name;
        hdl.creator = proc_id;
        hdl.flags = flags;

        Arena_t arena = translate_grow_direction(grow_direction(flags));

        AllocReturn_t alloc_return
            (dtmmgr_alloc (&m_mmgr, hdl.id, arena, size));

        DLOG( TRACE
            , "ALLOC:"
            << " handle = " << hdl.id
            << " arena = " << arena
            << " size = " << size
            << " return = " << alloc_return
            );

        switch (alloc_return)
        {
        case ALLOC_SUCCESS:
          {
            Offset_t offset (0);
            dtmmgr_offset_size ( m_mmgr
                               , hdl.id
                               , arena
                               , &offset
                               , NULL
                               );
            hdl.offset = offset;
            try
            {
              alloc_hook (hdl);
            }
            catch (std::exception const & ex)
            {
              LOG(ERROR, "alloc_hook failed: " << ex.what());
              dtmmgr_free (&m_mmgr, hdl.id, arena);
              throw;
            }

            update_descriptor_from_mmgr ();
            m_handles [hdl.id] = hdl;
            return hdl.id;
          }
          break;
        case ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY:
          LOG( WARN
             , "not enough contiguous memory available:"
             << " requested_size = " << size
             << " segment = " << m_descriptor.id
             << " avail = " << m_descriptor.avail
             );
          // TODO:
          //    defrag (size);
          //        release locks (? how)
          //          block all accesses to this area
          //              // memcpy/allocs should return EAGAIN
          //          wait for transactions to finish
          //          real_defrag
          //        reacquire locks
          throw std::runtime_error
              ("not enough contiguous memory");
          break;
        case ALLOC_INSUFFICIENT_MEMORY:
          LOG( ERROR
             , "not enough memory:"
             << " requested_size=" << size
             << " segment=" << m_descriptor.id
             << " avail=" << m_descriptor.avail
             );
          throw std::runtime_error ("out of memory");
          break;
        case ALLOC_DUPLICATE_HANDLE:
          LOG( ERROR
             ,  "duplicate handle:"
             << " handle = " << hdl.id
             << " segment " << m_descriptor.id
             );
          throw std::runtime_error ("duplicate handle");
          break;
        case ALLOC_FAILURE:
          LOG( ERROR
             , "internal error during allocation:"
             << " requested_size = " << size
             << " handle = " << hdl.id
             << " segment = " << m_descriptor.id
             );
          throw std::runtime_error ("allocation failed");
          break;
        default:
          LOG( ERROR
             ,  "unexpected error during allocation:"
             << " requested_size = " << size
             << " handle = " << hdl.id
             << " segment = " << m_descriptor.id
             << " error = " << alloc_return
             );
          throw std::runtime_error ("unexpected return code");
          break;
        }

        return hdl.id;
      }

      void area_t::defrag (const gpi::pc::type::size_t)
      {
        throw std::runtime_error ("defrag is not yet implemented");
      }

      void area_t::update_descriptor_from_mmgr()
      {
        m_descriptor.avail = dtmmgr_memfree (m_mmgr);
        m_descriptor.allocs =
            dtmmgr_numhandle (m_mmgr, ARENA_UP)
          + dtmmgr_numhandle (m_mmgr, ARENA_DOWN);
        // dtmmgr_numalloc -> total allocs
        // dtmmgr_numfree -> total frees
        m_descriptor.ts.touch();
      }

      void area_t::free (const gpi::pc::type::handle_t hdl)
      {
        lock_type lock (m_mutex);

        if (m_handles.find(hdl) == m_handles.end())
        {
          LOG( ERROR
             , "no such handle: "
             << " handle = " << hdl
             << " segment = " << m_descriptor.id
             );
          throw std::runtime_error ("no such handle");
        }

        const gpi::pc::type::handle::descriptor_t desc (m_handles.at(hdl));
        if (desc.nref)
        {
          LOG( WARN
             , "handle still in use:"
             << " handle = " << hdl
             << " nref = " << desc.nref
             );
          throw std::runtime_error ("handle still in use");
        }

        Arena_t arena (translate_grow_direction(grow_direction(desc.flags)));
        HandleReturn_t handle_return (dtmmgr_free ( &m_mmgr
                                                  , hdl
                                                  , arena
                                                  )
                                     );
        switch (handle_return)
        {
        case RET_SUCCESS:
          DLOG(TRACE, "handle free'd: " << desc);
          m_handles.erase (hdl);
          update_descriptor_from_mmgr ();
          try
          {
            free_hook (desc);
          }
          catch (std::exception const & ex)
          {
            LOG(ERROR, "free_hook failed: " << ex.what());
            throw;
          }
          break;
        case RET_HANDLE_UNKNOWN:
          LOG( ERROR
             , "***** INCONSISTENCY DETECTED *****:"
             << " mmgr did not know handle " << desc
             );
          throw std::runtime_error ("inconsistent state: no such handle");
          break;
        case RET_FAILURE:
          LOG( ERROR
             , "unknown error during free:"
             << " handle = " << desc
             );
          throw std::runtime_error ("no such handle");
          break;
        }
      }

      void area_t::remote_free (const gpi::pc::type::handle_t hdl)
      {
        lock_type lock (m_mutex);

        if (m_handles.find(hdl) == m_handles.end())
        {
          LOG( ERROR
             , "no such handle: "
             << " handle = " << hdl
             << " segment = " << m_descriptor.id
             );
          throw std::runtime_error ("no such handle");
        }

        const gpi::pc::type::handle::descriptor_t desc (m_handles.at(hdl));
        if (desc.nref)
        {
          LOG( WARN
             , "handle still in use:"
             << " handle = " << hdl
             << " nref = " << desc.nref
             );
        }

        Arena_t arena (translate_grow_direction(grow_direction(desc.flags)));
        HandleReturn_t handle_return (dtmmgr_free ( &m_mmgr
                                                  , hdl
                                                  , arena
                                                  )
                                     );
        switch (handle_return)
        {
        case RET_SUCCESS:
          DLOG(TRACE, "handle free'd: " << desc);
          m_handles.erase (hdl);
          update_descriptor_from_mmgr ();
          break;
        case RET_HANDLE_UNKNOWN:
          LOG( ERROR
             , "***** INCONSISTENCY DETECTED *****:"
             << " mmgr did not know handle " << desc
             );
          throw std::runtime_error ("inconsistent state: no such handle");
          break;
        case RET_FAILURE:
          LOG( ERROR
             , "unknown error during free:"
             << " handle = " << desc
             );
          throw std::runtime_error ("no such handle");
          break;
        }
      }

      gpi::pc::type::segment::descriptor_t const &
      area_t::descriptor () const
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
          LOG(ERROR, "cannot find descriptor for handle " << hdl);
          throw std::runtime_error ("no such handle");
        }
      }

      void area_t::list_allocations (gpi::pc::type::handle::list_t & list) const
      {
        lock_type lock (m_mutex);

        for ( handle_descriptor_map_t::const_iterator pos (m_handles.begin())
            ; pos != m_handles.end()
            ; ++pos
            )
        {
          list.push_back (pos->second);
        }
      }

      gpi::pc::type::size_t
      area_t::attach_process (const gpi::pc::type::process_id_t id)
      {
        lock_type lock (m_mutex);
        if (m_attached_processes.insert (id).second)
        {
          ++m_descriptor.nref;
        }
        return m_descriptor.nref;
      }

      gpi::pc::type::size_t
      area_t::detach_process (const gpi::pc::type::process_id_t id)
      {
        lock_type lock (m_mutex);
        process_ids_t::iterator p (m_attached_processes.find(id));
        if (p != m_attached_processes.end())
        {
          --m_descriptor.nref;
          m_attached_processes.erase (p);
        }
        return m_descriptor.nref;
      }

      void *
      area_t::pointer_to (gpi::pc::type::memory_location_t const &loc)
      {
        lock_type lock (m_mutex);

        handle_descriptor_map_t::const_iterator hdl_it
            (m_handles.find(loc.handle));
        if (hdl_it == m_handles.end())
          throw std::runtime_error
            ("pointer_to(): no such handle: " + boost::lexical_cast<std::string>(loc.handle));

        return reinterpret_cast<char*>(ptr())
          + (hdl_it->second.offset + (loc.offset % hdl_it->second.size));
      }
    }
  }
}
