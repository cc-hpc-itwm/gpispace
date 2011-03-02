#include "manager.hpp"

#include <fhglog/minimal.hpp>

#include <gpi-space/gpi/api.hpp>

#include "handle_generator.hpp"
#include "shm_area.hpp"
#include "gpi_area.hpp"

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

        add_gpi_memory ();
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
        while (! m_areas.empty())
        {
          unregister_memory (m_areas.begin()->first);
        }
      }

      gpi::pc::type::segment_id_t
      manager_t::register_memory ( const gpi::pc::type::process_id_t creator
                                 , const std::string & name
                                 , const gpi::pc::type::size_t size
                                 , const gpi::pc::type::flags_t flags
                                 )
      {
        gpi::pc::type::segment_id_t id (m_segment_counter.inc());

        {
          lock_type lock (m_mutex);
          area_ptr area (new shm_area_t ( id
                                        , creator
                                        , name
                                        , size
                                        , flags
                                        )
                         );
          m_areas[id] = area;
          LOG(TRACE, "memory segment registered: " << area->descriptor ());
        }
        memory_added (id);
        return id;
      }

      void
      manager_t::unregister_memory (const gpi::pc::type::segment_id_t mem_id)
      {
        {
          lock_type lock (m_mutex);

          area_map_t::iterator area_it (m_areas.find(mem_id));
          if (area_it == m_areas.end())
          {
            throw std::runtime_error ("no such memory");
          }

          area_ptr area (area_it->second);

          if (area->in_use ())
          {
            LOG(WARN, "memory area is still in use: " << area->descriptor());

            // TODO: maybe move memory segment to garbage area

            throw std::runtime_error
                ("segment is still inuse, cannot unregister");
          }

          // WORK HERE:
          //    let this do another thread
          //    and just give him the area_ptr
          area_it->second->garbage_collect ();
          m_areas.erase (area_it);
          LOG(TRACE, "memory removed: " << mem_id);
        }

        memory_removed (mem_id);
      }

      void manager_t::list_memory (gpi::pc::type::segment::list_t &l) const
      {
        lock_type lock (m_mutex);

        for ( area_map_t::const_iterator area (m_areas.begin())
            ; area != m_areas.end()
            ; ++area
            )
        {
          l.push_back (area->second->descriptor());
        }
      }

      void
      manager_t::attach_process ( const gpi::pc::type::process_id_t proc_id
                                , const gpi::pc::type::segment_id_t mem_id
                                )
      {
        lock_type lock (m_mutex);
        area_map_t::iterator area (m_areas.find(mem_id));
        if (area == m_areas.end())
        {
          throw std::runtime_error ("no such memory area");
        }
        area->second->attach_process (proc_id);
        process_attached(mem_id, proc_id);
      }

      void
      manager_t::detach_process ( const gpi::pc::type::process_id_t proc_id
                                , const gpi::pc::type::segment_id_t mem_id
                                )
      {
        lock_type lock (m_mutex);
        area_map_t::iterator area (m_areas.find(mem_id));
        if (area == m_areas.end())
        {
          throw std::runtime_error ("no such memory area");
        }
        area->second->detach_process (proc_id);
        process_detached(mem_id, proc_id);

        if (area->second->is_eligible_for_deletion())
        {
          unregister_memory (mem_id);
        }
      }

      void
      manager_t::garbage_collect (const gpi::pc::type::process_id_t proc_id)
      {
        lock_type lock (m_mutex);
        std::list<gpi::pc::type::segment_id_t> segments;
        for ( area_map_t::iterator area (m_areas.begin())
            ; area != m_areas.end()
            ; ++area
            )
        {
          segments.push_back (area->first);
        }

        while (! segments.empty())
        {
          detach_process (proc_id, segments.front());
          segments.pop_front();
        }
      }

      void
      manager_t::add_gpi_memory ()
      {
        lock_type lock (m_mutex);

        gpi::pc::type::segment_id_t id (m_segment_counter.inc());

        if (m_areas.find(id) != m_areas.end())
        {
          throw std::runtime_error
              ("cannot add another gpi segment: id already in use!");
        }

        area_ptr area
            (new gpi_area_t
             ( id, 0, "GPI"
             , gpi::api::gpi_api_t::get().memory_size ()
             , gpi::pc::type::segment::F_PERSISTENT
             | gpi::pc::type::segment::F_NOUNLINK
             , gpi::api::gpi_api_t::get().dma_ptr ()
             )
            );

        m_areas[id] = area;

        LOG(TRACE, "GPI memory registered:" << area->descriptor ());

        memory_added (id);
      }

      manager_t::area_ptr
      manager_t::get_area (const gpi::pc::type::segment_id_t mem_id)
      {
        lock_type lock (m_mutex);
        area_map_t::iterator area_it (m_areas.find (mem_id));
        if (area_it == m_areas.end())
        {
          throw std::runtime_error ("no such memory");
        }
        return area_it->second;
      }

      manager_t::area_ptr
      manager_t::get_area (const gpi::pc::type::segment_id_t mem_id) const
      {
        lock_type lock (m_mutex);
        area_map_t::const_iterator area_it (m_areas.find (mem_id));
        if (area_it == m_areas.end())
        {
          throw std::runtime_error ("no such memory");
        }
        return area_it->second;
      }

      manager_t::area_ptr
      manager_t::get_area_by_handle (const gpi::pc::type::handle_t hdl)
      {
        lock_type lock (m_mutex);

        handle_to_segment_t::const_iterator h2s (m_handle_to_segment.find(hdl));
        if (h2s != m_handle_to_segment.end())
        {
          return get_area (h2s->second);
        }
        else
        {
          throw std::runtime_error ("no such handle");
        }
      }

      void
      manager_t::add_handle ( const gpi::pc::type::handle_t hdl
                            , const gpi::pc::type::segment_id_t seg_id
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
      manager_t::del_handle (const gpi::pc::type::handle_t hdl)
      {
        if (m_handle_to_segment.find (hdl) == m_handle_to_segment.end())
        {
          throw std::runtime_error ("handle does not exist");
        }
        m_handle_to_segment.erase (hdl);
      }

      gpi::pc::type::handle_t
      manager_t::alloc ( const gpi::pc::type::process_id_t proc_id
                       , const gpi::pc::type::segment_id_t seg_id
                       , const gpi::pc::type::size_t size
                       , const std::string & name
                       , const gpi::pc::type::flags_t flags
                       )
      {
        area_ptr area (get_area (seg_id));

        assert (area);

        gpi::pc::type::handle_t hdl (area->alloc (proc_id, size, name, flags));

        add_handle (hdl, seg_id);

        handle_allocated (hdl);

        return hdl;
      }

      void
      manager_t::free (const gpi::pc::type::handle_t hdl)
      {
        area_ptr area (get_area_by_handle (hdl));

        assert (area);

        area->free (hdl);
        del_handle (hdl);

        handle_freed (hdl);
      }

      void
      manager_t::list_allocations( const gpi::pc::type::segment_id_t id
                                 , gpi::pc::type::handle::list_t & l
                                 ) const
      {
        get_area (id)->list_allocations (l);
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

      gpi::pc::type::queue_id_t
      manager_t::memcpy ( const gpi::pc::type::process_id_t proc_id
                        , gpi::pc::type::memory_location_t const & dst
                        , gpi::pc::type::memory_location_t const & src
                        , const gpi::pc::type::size_t amount
                        , const gpi::pc::type::queue_id_t queue
                        )
      {
        lock_type lock (m_mutex);

//        check_permissions (permission::memcpy_t (proc_id, dst, src));
//        check_boundaries  (dst, src, amount);
        LOG( TRACE
           , "process " << proc_id
           << " requesting transfer"
           << " of " << amount << " bytes"
           << " from " << src
           << " to " << dst
           << " via queue " << queue
           );
        throw std::runtime_error ("not yet implemented");
      }
    }
  }
}
