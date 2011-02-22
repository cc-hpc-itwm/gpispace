#include "manager.hpp"

#include <fhglog/minimal.hpp>

#include "handle_generator.hpp"

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      manager_t::manager_t (const gpi::pc::type::id_t ident)
        : m_ident (ident)
        , m_segment_counter ()
      {
        handle_generator_t::create (ident);
      }

      manager_t::~manager_t ()
      {
        try
        {
          clear ();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not clear memory manager: " << ex.what());
        }
        handle_generator_t::destroy ();
      }

      void
      manager_t::clear ()
      {
        // preconditions:
        // make sure that there are no remaining
        // accesses to segments queued

        //     i.e. cancel/remove all items in the memory transfer component
        lock_type lock (m_mutex);
        while (! m_segments.empty())
        {
          unregister_memory (m_segments.begin()->first);
        }
      }

      gpi::pc::type::segment_id_t
      manager_t::register_memory( const gpi::pc::type::process_id_t creator
                     , const std::string & name
                     , const gpi::pc::type::size_t size
                     , const gpi::pc::type::flags_t flags
                     )
      {
        try
        {
          memory_ptr seg (new gpi::pc::segment::segment_t (name, size));

          seg->open ();

          gpi::pc::type::segment_id_t id
              (++m_segment_counter);

          seg->assign_id (id);
          seg->descriptor().creator = creator;
          seg->descriptor().flags = flags;

          // important: if F_EXCLUSIVE is set, F_NOUNLINK does not make any sense
          if (seg->descriptor().flags & gpi::pc::type::segment::F_EXCLUSIVE)
          {
            seg->unlink();
          }

          lock_type lock (m_mutex);
          m_segments [id] = seg;
          m_areas[id] = area_ptr(new area_t(seg));

          LOG(TRACE, "shared memory segment registered: " << seg->name() << " (" << seg->id() << ") size " << seg->size() << " @" << seg->ptr());

          memory_added (id);
          return id;
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not register memory segment: " << ex.what());
          throw;
        }
      }

      void manager_t::unregister_memory (const gpi::pc::type::segment_id_t seg_id)
      {
        lock_type lock (m_mutex);

        segment_map_t::iterator seg_it (m_segments.find(seg_id));
        if (seg_it == m_segments.end())
        {
          LOG(WARN, "tried to remove unknown memory segment: " << seg_id);
          return;
        }

        memory_ptr seg (seg_it->second);

        if (seg->descriptor().nref)
        {
          throw std::runtime_error ("segment is still inuse, cannot unregister");
        }

        if (!gpi::flag::is_set
              ( seg->descriptor().flags
              , gpi::pc::type::segment::F_NOUNLINK
              | gpi::pc::type::segment::F_EXCLUSIVE
              )
           )
        {
          seg->unlink();
        }

        m_segments.erase (seg_it);
        m_areas.erase(seg_id);

        LOG(TRACE, "memory segment removed: " << seg_id);

        memory_removed (seg_id);

        // TODO: maybe move memory segment to garbage area
      }

      void manager_t::list_memory (gpi::pc::type::segment::list_t &l) const
      {
        lock_type lock (m_mutex);

        for ( segment_map_t::const_iterator seg (m_segments.begin())
            ; seg != m_segments.end()
            ; ++seg
            )
        {
          l.push_back (seg->second->descriptor());
        }
      }

      gpi::pc::type::size_t
      manager_t::increment_refcount (const gpi::pc::type::segment_id_t seg_id)
      {
        lock_type lock (m_mutex);
        segment_map_t::iterator seg_it (m_segments.find(seg_id));
        if (seg_it == m_segments.end())
        {
          throw std::runtime_error
              ("cannot increment reference count of unknown memory");
        }
        return ++(seg_it->second->descriptor().nref);
      }

      gpi::pc::type::size_t
      manager_t::decrement_refcount (const gpi::pc::type::segment_id_t seg_id)
      {
        lock_type lock (m_mutex);
        segment_map_t::iterator seg_it (m_segments.find(seg_id));
        if (seg_it == m_segments.end())
        {
          throw std::runtime_error
              ("cannot decrement reference count of unknown memory");
        }
        memory_ptr seg (seg_it->second);
        if (0 == seg->descriptor().nref)
        {
          throw std::runtime_error
              ("cannot decrement reference count below 0");
        }
        gpi::pc::type::size_t ref_cnt
            (--(seg->descriptor().nref));

        if (0 == ref_cnt)
        {
          if (! gpi::flag::is_set ( seg->descriptor().flags
                                  , gpi::pc::type::segment::F_PERSISTENT
                                  )
             )
          {
            unregister_memory (seg_id);
          }
          /*
          else if (seg->descriptor().flags & gpi::pc::type::segment::F_EXCLUSIVE)
          {
            unregister_segment (seg_id);
          }
          */
        }

        return ref_cnt;
      }

      void
      manager_t::add_special_memory ( std::string const & name
                                    , const gpi::pc::type::segment_id_t id
                                    , const gpi::pc::type::size_t size
                                    , void *ptr
                                    )
      {
        gpi::pc::type::segment::descriptor_t desc;
        gpi::flag::set ( desc.flags
                       , gpi::pc::type::segment::F_SPECIAL | gpi::pc::type::segment::F_NOUNLINK
                       );
        desc.id = id;
        desc.creator = 0;
        desc.name = name;
        desc.size = size;
        desc.avail = size;
        desc.allocs = 0;
        desc.nref = 0;

        memory_ptr seg (new gpi::pc::segment::segment_t (desc, ptr));

        lock_type lock (m_mutex);
        if (m_segments.find(id) != m_segments.end())
        {
          throw std::runtime_error
              ("cannot add special segment: id already in use!");
        }

        m_segments [id] = seg;
        m_areas[id] = area_ptr(new area_t(seg));

        // move counter if required
        m_segment_counter.move_to (id);

        LOG(TRACE, "special memory segment registered: "
            << name << " (" << id << ")"
            << " size " << size
            << " @" << ptr
            );

        memory_added (id);
      }

      manager_t::memory_ptr
      manager_t::get_memory (const gpi::pc::type::segment_id_t seg_id)
      {
        lock_type lock (m_mutex);
        segment_map_t::iterator seg_it (m_segments.find(seg_id));
        if (seg_it == m_segments.end())
        {
          throw std::runtime_error
              ("no such memory segment");
        }
        return seg_it->second;
      }

      gpi::pc::type::handle_id_t
      manager_t::alloc ( const gpi::pc::type::process_id_t proc_id
                       , const gpi::pc::type::segment_id_t seg_id
                       , const gpi::pc::type::size_t size
                       , const std::string & name
                       , const gpi::pc::type::flags_t flags
                       )
      {
        lock_type lock (m_mutex);

        // F_GLOBAL flag:
        //   on shared segment
        //     allocation is accessible by others
        //   on gpi segment
        //     global allocation
        // quick hack for globa/local allocations
        gpi::pc::type::flags_t new_flags (flags);
        if (seg_id == gpi::pc::type::segment::SEG_GLOBAL)
        {
          throw std::runtime_error
            ("global allocations not yet implemented");
        }
        else if (seg_id == gpi::pc::type::segment::SEG_LOCAL)
        {
          // sanity check
          gpi::flag::clear
            (new_flags, gpi::pc::type::handle::F_GLOBAL);
        }
        area_map_t::iterator area_iter
            (m_areas.find (seg_id));
        if (area_iter == m_areas.end())
        {
          LOG(WARN, "alloc from process " << proc_id << " failed: no such area: " << seg_id);
          throw std::runtime_error ("no such memory area");
        }

        gpi::pc::type::handle_id_t hdl
            (area_iter->second->alloc (proc_id, size, name, new_flags));
        m_handle_to_segment [hdl] = seg_id;
        return hdl;
      }

      void
      manager_t::free (const gpi::pc::type::handle_id_t hdl_id)
      {
        lock_type lock (m_mutex);

        gpi::pc::type::handle_t hdl (hdl_id);

        handle_to_segment_t::iterator hdl_to_seg (m_handle_to_segment.find(hdl));
        if (hdl_to_seg == m_handle_to_segment.end())
        {
          LOG(WARN, "free of unknown handle: " << hdl);
          throw std::runtime_error ("unknown handle");
        }

        area_map_t::iterator seg_to_area (m_areas.find (hdl_to_seg->second));
        if (seg_to_area == m_areas.end())
        {
          LOG( ERROR
             , "*** INCONSISTENCY DETECTED ***:"
               << " handle " << hdl
               << " maps to unknown segment " << hdl_to_seg->second
             );
          m_handle_to_segment.erase (hdl);
          throw std::runtime_error ("unknown area for handle");
        }

        seg_to_area->second->free (hdl);
      }

      void
      manager_t::list_allocations( const gpi::pc::type::segment_id_t seg
                                 , gpi::pc::type::handle::list_t & l
                                 ) const
      {
        lock_type lock (m_mutex);

        area_map_t::const_iterator seg_to_area (m_areas.find (seg));
        if (seg_to_area == m_areas.end())
        {
          LOG( WARN, "listing of unknown memory area " << seg << " requested");
          throw std::runtime_error ("no such area");
        }
        seg_to_area->second->list_allocations (l);
      }

      void
      manager_t::list_allocations(gpi::pc::type::handle::list_t & l) const
      {
        lock_type lock (m_mutex);

        for ( area_map_t::const_iterator s2a (m_areas.begin())
            ; s2a != m_areas.end()
            ; ++s2a
            )
        {
          s2a->second->list_allocations (l);
        }
      }
    }
  }
}
