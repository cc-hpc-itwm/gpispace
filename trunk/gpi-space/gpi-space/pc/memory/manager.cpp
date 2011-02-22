#include "manager.hpp"

#include <fhglog/minimal.hpp>

#include "handle_generator.hpp"

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      manager_t::manager_t ( const gpi::pc::type::id_t ident
                           , gpi::pc::segment::manager_t & segment_mgr
                           )
        : m_ident (ident)
        , m_segment_mgr (segment_mgr)
      {
        handle_generator_t::create (ident);
      }

      manager_t::~manager_t ()
      {
        handle_generator_t::destroy ();
      }

      void
      manager_t::add_area_for_segment (const gpi::pc::type::segment_id_t seg_id)
      {
        lock_type lock (m_mutex);
        if (m_areas.find (seg_id) != m_areas.end())
        {
          throw std::runtime_error ("area already registered for segment");
        }

        m_areas[seg_id] =
            area_ptr(new area_t(m_segment_mgr.get_segment (seg_id)));

        DLOG(TRACE, "added memory area for segment " << seg_id);
      }

      void
      manager_t::del_area_for_segment (const gpi::pc::type::segment_id_t seg_id)
      {
        lock_type lock (m_mutex);
        m_areas.erase (seg_id);

        DLOG(TRACE, "removed memory area for segment " << seg_id);
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
          throw std::runtime_error ("no such area");
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
