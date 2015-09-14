#include <fhg/assert.hpp>
#include <gpi-space/pc/url.hpp>
#include <gpi-space/pc/url_io.hpp>

#include <fhglog/LogMacros.hpp>

#include <gpi-space/gpi/api.hpp>
#include <gpi-space/pc/global/topology.hpp>
#include <gpi-space/pc/memory/gpi_area.hpp>
#include <gpi-space/pc/memory/handle_generator.hpp>
#include <gpi-space/pc/memory/manager.hpp>
#include <gpi-space/pc/memory/memory_area.hpp>
#include <gpi-space/pc/memory/memory_transfer_t.hpp>
#include <gpi-space/pc/memory/sfs_area.hpp>
#include <gpi-space/pc/memory/shm_area.hpp>

#include <string>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      namespace
      {
        area_ptr_t create_area ( fhg::log::Logger& logger
                               , std::string const &url_s
                               , global::topology_t& topology
                               , handle_generator_t& handle_generator
                               , api::gpi_api_t& gpi_api
                               )
      {
        url_t url (url_s);

        return
          ( url.type() == "gpi" ? gpi_area_t::create (logger, url_s, topology, handle_generator, gpi_api)
          : url.type() == "sfs" ? sfs_area_t::create (logger, url_s, topology, handle_generator)
          : url.type() == "shm" ? shm_area_t::create (logger, url_s, handle_generator)
          : throw std::runtime_error
              ("no memory type registered with: '" + url_s + "'")
          );
      }
      }

      manager_t::manager_t (fhg::log::Logger& logger, api::gpi_api_t& gpi_api)
        : _logger (logger)
        , _gpi_api (gpi_api)
        , m_transfer_mgr (_logger, gpi_api)
        , _handle_generator (gpi_api.rank())
      {
        _handle_generator.initialize_counter
          (gpi::pc::type::segment::SEG_INVAL, MAX_PREALLOCATED_SEGMENT_ID);
      }

      manager_t::~manager_t ()
      {
        try
        {
          clear ();
        }
        catch (std::exception const & ex)
        {
          LLOG( ERROR
              , _logger
             , "could not clear memory manager: " << ex.what()
             );
        }
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

      handle_generator_t& manager_t::handle_generator()
      {
        return _handle_generator;
      }

      gpi::pc::type::segment_id_t
      manager_t::register_memory ( const gpi::pc::type::process_id_t creator
                                 , const area_ptr &area
                                 )
      {
        add_area (area);
        attach_process (creator, area->get_id ());

        return area->get_id ();
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
            throw std::runtime_error ( "no such memory: "
                                     + boost::lexical_cast<std::string>(mem_id)
                                     );
          }

          area_ptr area (area_it->second);

          if (area->in_use ())
          {
            // TODO: maybe move memory segment to garbage area

            throw std::runtime_error
                ("segment is still inuse, cannot unregister");
          }

          // WORK HERE:
          //    let this do another thread
          //    and just give him the area_ptr
          area->garbage_collect ();
          m_areas.erase (area_it);

          LLOG(TRACE, _logger, "memory removed: " << mem_id);
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
          throw std::runtime_error ( "no such memory: "
                                   + boost::lexical_cast<std::string>(mem_id)
                                   );
        }

        if (proc_id)
        {
          area->second->attach_process (proc_id);
        }
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
          throw std::runtime_error ( "no such memory: "
                                   + boost::lexical_cast<std::string>(mem_id)
                                   );
        }

        if (proc_id)
        {
          area->second->detach_process (proc_id);

          if (area->second->is_eligible_for_deletion())
          {
            unregister_memory (mem_id);
          }
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
      manager_t::add_area (manager_t::area_ptr const &area)
      {
        lock_type lock (m_mutex);

        if (area->get_id () == (gpi::pc::type::id_t (-1)))
        {
          area->set_id
            (_handle_generator.next (gpi::pc::type::segment::SEG_INVAL));
        }
        else
        {
          if (m_areas.find (area->get_id ()) != m_areas.end())
          {
            throw std::runtime_error
              ("cannot add another gpi segment: id already in use!");
          }
        }

        m_areas [area->get_id ()] = area;
        area->init ();
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
          throw std::runtime_error ( "no such memory: "
                                   + boost::lexical_cast<std::string>(mem_id)
                                   );
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
          throw std::runtime_error ( "no such handle: "
                                   + boost::lexical_cast<std::string>(hdl)
                                   );
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
        area_ptr area (get_area (seg_id));
        area->remote_alloc (hdl, offset, size, local_size, name);
        add_handle (hdl, seg_id);

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

        fhg_assert (area);

        gpi::pc::type::handle_t hdl (area->alloc (proc_id, size, name, flags));

        add_handle (hdl, seg_id);

        return hdl;
      }

      void
      manager_t::remote_free (const gpi::pc::type::handle_t hdl)
      {
        area_ptr area (get_area_by_handle (hdl));

        fhg_assert (area);

        area->remote_free (hdl);
        del_handle (hdl);
      }

      void
      manager_t::free (const gpi::pc::type::handle_t hdl)
      {
        area_ptr area (get_area_by_handle (hdl));

        fhg_assert (area);

        del_handle (hdl);
        area->free (hdl);
      }

      gpi::pc::type::handle::descriptor_t
      manager_t::info (const gpi::pc::type::handle_t hdl) const
      {
        return get_area_by_handle(hdl)->descriptor(hdl);
      }

      std::map<std::string, double>
      manager_t::get_transfer_costs (const std::list<gpi::pc::type::memory_region_t>& transfers) const
      {
        std::map<std::string, double> costs;

        for (gpi::pc::type::memory_region_t const& transfer : transfers)
        {
          const area_ptr area (get_area_by_handle (transfer.location.handle));

          for (gpi::rank_t rank = 0; rank < _gpi_api.number_of_nodes(); ++rank)
          {
            costs[_gpi_api.hostname_of_rank (rank)] += area->get_transfer_costs (transfer, rank);
          }
        }

        return costs;
      }

      gpi::pc::type::queue_id_t
      manager_t::memcpy ( gpi::pc::type::memory_location_t const & dst
                        , gpi::pc::type::memory_location_t const & src
                        , const gpi::pc::type::size_t amount
                        )
      {
        memory_transfer_t t;
        t.dst_area     = get_area_by_handle(dst.handle);
        t.dst_location = dst;
        t.src_area     = get_area_by_handle(src.handle);
        t.src_location = src;
        t.amount       = amount;
          static gpi::pc::type::queue_id_t rr_queue = 0;
          t.queue        = rr_queue;
          rr_queue = (rr_queue + 1) % m_transfer_mgr.num_queues ();

        t.dst_area->check_bounds (dst, amount);
        t.src_area->check_bounds (src, amount);
//        check_permissions (permission::memcpy_t (proc_id, dst, src));

        // TODO: increase refcount in handles, set access/modification times
        m_transfer_mgr.transfer (t);

        return t.queue;
      }

      gpi::pc::type::size_t
      manager_t::wait_on_queue ( const gpi::pc::type::process_id_t
                               , const gpi::pc::type::queue_id_t queue
                               )
      {
        return m_transfer_mgr.wait_on_queue (queue);
      }

      int
      manager_t::remote_add_memory ( const gpi::pc::type::segment_id_t seg_id
                                   , std::string const & url
                                   , global::topology_t& topology
                                   )
      {
        area_ptr_t area (create_area (_logger, url, topology, _handle_generator, _gpi_api));
        area->set_owner (0);
        area->set_id (seg_id);
        add_area (area);
        return 0;
      }

      gpi::pc::type::segment_id_t
      manager_t::add_memory ( const gpi::pc::type::process_id_t proc_id
                            , const std::string & url_s
                            , const gpi::pc::type::segment_id_t seg_id
                            , global::topology_t& topology
                            )
      {
        area_ptr_t area (create_area (_logger, url_s, topology, _handle_generator, _gpi_api));
        area->set_owner (proc_id);
        if (seg_id > 0)
          area->set_id (seg_id);

        add_area (area);

        if (area->flags () & F_GLOBAL)
        {
          try
          {
            url_t old_url (url_s);
            url_t new_url (old_url.type(), old_url.path());
            new_url.set ("persistent", "true");
            topology.add_memory
              (area->get_id (), boost::lexical_cast<std::string>(new_url));
          }
          catch (std::exception const & up)
          {
            try
            {
              del_memory (proc_id, area->get_id (), topology);
            }
            catch (...)
            {
              // ignore follow up exception
            }

            throw;
          }
        }

        return area->get_id ();
      }

      int
      manager_t::remote_del_memory ( const gpi::pc::type::segment_id_t seg_id
                                   , global::topology_t& topology
                                   )
      {
        del_memory (0, seg_id, topology);
        return 0;
      }

      void
      manager_t::del_memory ( const gpi::pc::type::process_id_t proc_id
                            , const gpi::pc::type::segment_id_t seg_id
                            , global::topology_t& topology
                            )
      {
        if (0 == seg_id)
          throw std::runtime_error ("invalid segment id");
        if (seg_id <= gpi::pc::memory::manager_t::MAX_PREALLOCATED_SEGMENT_ID)
          throw std::runtime_error ("permission denied");

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

          if (area->in_use ())
          {
            // TODO: maybe move memory segment to garbage area

            throw std::runtime_error
              ("segment is still inuse, cannot unregister");
          }

          area->garbage_collect ();
          m_areas.erase (area_it);

          if (proc_id > 0 && area->flags () & F_GLOBAL)
          {
            topology.del_memory (seg_id);
          }
        }
      }
    }
  }
}
