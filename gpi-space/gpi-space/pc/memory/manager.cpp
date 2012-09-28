#include "manager.hpp"

#include <fhglog/minimal.hpp>
#include <fhg/assert.hpp>

#include <gpi-space/gpi/api.hpp>

#include "memory_transfer_t.hpp"
#include "handle_generator.hpp"
#include "shm_area.hpp"
#include "gpi_area.hpp"

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      manager_t::manager_t ()
        : m_ident (gpi::api::gpi_api_t::get().rank())
        , m_segment_counter ()
        , m_transfer_mgr(gpi::api::gpi_api_t::get().number_of_queues())
      {
        handle_generator_t::create (m_ident);

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
          CLOG( ERROR
              , "gpi.memory"
              , "could not clear memory manager: " << ex.what()
              );
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
          CLOG( TRACE
              , "gpi.memory"
              , "memory segment registered: " << area->descriptor ()
              );
        }
        memory_added (id);

        attach_process (creator, id);

        return id;
      }

      void
      manager_t::unregister_memory ( const gpi::pc::type::process_id_t pid
                                   , const gpi::pc::type::segment_id_t mem_id
                                   )
      {
        lock_type lock (m_mutex);

        detach_process (pid, mem_id);
        if (m_areas.find(mem_id) != m_areas.end())
        {
          try
          {
            unregister_memory(mem_id);
          }
          catch (...)
          {
            attach_process(pid, mem_id); // unroll
            throw;
          }
        }
      }

      void
      manager_t::unregister_memory (const gpi::pc::type::segment_id_t mem_id)
      {
        {
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
          area->second->garbage_collect (proc_id);

          if (area->second->is_process_attached (proc_id))
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
        return static_cast<const manager_t*>(this)->get_area (mem_id);
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
        return static_cast<const manager_t*>(this)->get_area_by_handle(hdl);
      }

      manager_t::area_ptr
      manager_t::get_area_by_handle (const gpi::pc::type::handle_t hdl) const
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
        lock_type lock (m_mutex);
        if (m_handle_to_segment.find (hdl) == m_handle_to_segment.end())
        {
          throw std::runtime_error ("handle does not exist");
        }
        m_handle_to_segment.erase (hdl);
      }

      int
      manager_t::remote_alloc ( const gpi::pc::type::segment_id_t seg_id
                              , const gpi::pc::type::handle_t hdl
                              , const gpi::pc::type::offset_t offset
                              , const gpi::pc::type::size_t size
                              , const gpi::pc::type::size_t local_size
                              , const std::string & name
                              )
      {
        try
        {
          area_ptr area (get_area (seg_id));
          area->remote_alloc (hdl, offset, size, local_size, name);
          add_handle (hdl, seg_id);
          handle_allocated (hdl);

          DLOG( TRACE
              , "remote memory allocated:"
              << " segment " << seg_id
              << " size " << size
              << " local " << local_size
              << " handle " << hdl
              );
        }
        catch (std::exception const & ex)
        {
          // TODO: check error
          LOG(ERROR, "remote allocation failed: " << ex.what());
          return 2;
        }
        return 0;
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

        DLOG( TRACE
            , "memory allocated:"
            << " process " << proc_id
            << " segment " << seg_id
            << " size " << size
            << " handle " << hdl
            );

        return hdl;
      }

      void
      manager_t::remote_free (const gpi::pc::type::handle_t hdl)
      {
        area_ptr area (get_area_by_handle (hdl));

        assert (area);

        area->remote_free (hdl);
        del_handle (hdl);

        handle_freed (hdl);

        DLOG( TRACE
            , "remote memory deallocated:"
            << " handle " << hdl
            );
      }

      void
      manager_t::free (const gpi::pc::type::handle_t hdl)
      {
        area_ptr area (get_area_by_handle (hdl));

        assert (area);

        del_handle (hdl);
        area->free (hdl);

        handle_freed (hdl);

        DLOG( TRACE
            , "memory deallocated:"
            << " handle " << hdl
            );
      }

      gpi::pc::type::handle::descriptor_t
      manager_t::info (const gpi::pc::type::handle_t hdl) const
      {
        return get_area_by_handle(hdl)->descriptor(hdl);
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
      manager_t::memcpy ( const gpi::pc::type::process_id_t pid
                        , gpi::pc::type::memory_location_t const & dst
                        , gpi::pc::type::memory_location_t const & src
                        , const gpi::pc::type::size_t amount
                        , const gpi::pc::type::queue_id_t queue
                        )
      {
        memory_transfer_t t;
        t.pid          = pid;
        t.dst_area     = get_area_by_handle(dst.handle);
        t.dst_location = dst;
        t.src_area     = get_area_by_handle(src.handle);
        t.src_location = src;
        t.amount       = amount;
        t.queue        = queue;

        DLOG ( TRACE
             , "process " << pid
             << " requesting transfer "
             << t
             );

//        check_permissions (permission::memcpy_t (proc_id, dst, src));
        check_boundaries(dst, src, amount);

        // TODO: increase refcount in handles, set access/modification times
        m_transfer_mgr.transfer(t);
        return queue;
      }

      gpi::pc::type::size_t
      manager_t::wait_on_queue ( const gpi::pc::type::process_id_t proc_id
                               , const gpi::pc::type::queue_id_t queue
                               )
      {
        DLOG(TRACE, "wait_on_queue(" << queue << ") by process " << proc_id);
        return m_transfer_mgr.wait_on_queue (queue);
      }

      void
      manager_t::check_boundaries ( const gpi::pc::type::memory_location_t &dst
                                  , const gpi::pc::type::memory_location_t &src
                                  , const gpi::pc::type::size_t amount
                                  ) const
      {
        get_area_by_handle (dst.handle)->check_bounds (dst, amount);
        get_area_by_handle (src.handle)->check_bounds (src, amount);
      }
    }
  }
}
