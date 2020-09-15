#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/finally.hpp>
#include <util-generic/functor_visitor.hpp>
#include <util-generic/print_exception.hpp>

#include <iml/vmem/gaspi/pc/global/topology.hpp>
#include <iml/vmem/gaspi/pc/memory/gaspi_area.hpp>
#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>
#include <iml/vmem/gaspi/pc/memory/manager.hpp>
#include <iml/vmem/gaspi/pc/memory/memory_area.hpp>
#include <iml/vmem/gaspi/pc/memory/beegfs_area.hpp>
#include <iml/vmem/gaspi/pc/memory/shm_area.hpp>

#include <string>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      namespace
      {
        area_ptr_t create_area ( iml::segment_description const& description
                               , unsigned long total_size
                               , global::topology_t& topology
                               , handle_generator_t& handle_generator
                               , fhg::iml::vmem::gaspi_context& gaspi_context
                               , type::id_t owner
                               )
        {
          return fhg::util::visit<area_ptr_t>
            ( description
            , [&] (iml::gaspi_segment_description const& desc)
              {
                return gaspi_area_t::create ( desc
                                            , total_size
                                            , topology
                                            , handle_generator
                                            , gaspi_context
                                            , owner
                                            );
              }
            , [&] (iml::beegfs_segment_description const& desc)
              {
                return beegfs_area_t::create
                  (desc, total_size, topology, handle_generator, owner);
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
        _handle_generator.initialize_counter
          (gpi::pc::type::segment::SEG_INVAL);

        const std::size_t number_of_queues (gaspi_context.number_of_queues());
        for (std::size_t i (0); i < number_of_queues; ++i)
        {
          _task_threads.emplace_back
            ( fhg::util::cxx14::make_unique<boost::strict_scoped_thread<>>
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
        }
      }

      handle_generator_t& manager_t::handle_generator()
      {
        return _handle_generator;
      }

      std::pair<type::segment_id_t, type::handle_t>
        manager_t::register_shm_segment_and_allocate
          ( gpi::pc::type::process_id_t creator
          , std::shared_ptr<shm_area_t> area
          , gpi::pc::type::size_t size
          , std::string const& name
          )
      {
        auto const segment
          (_handle_generator.next (gpi::pc::type::segment::SEG_INVAL));
        area->set_id (segment);
        add_area (area);
      {
        lock_type lock (m_mutex);
        area_map_t::iterator area (m_areas.find(segment));
        if (area == m_areas.end())
        {
          throw std::runtime_error ( "no such memory: "
                                   + boost::lexical_cast<std::string>(segment)
                                   );
        }

        if (creator)
        {
          area->second->attach_process (creator);
        }
      }

        auto const allocation
          (area->alloc (creator, size, name, is_global::no));
        add_handle (allocation, segment);

        return {segment, allocation};
      }

      void
      manager_t::unregister_memory ( const gpi::pc::type::process_id_t pid
                                   , const gpi::pc::type::segment_id_t mem_id
                                   )
      {
        lock_type lock (m_mutex);

      {
        lock_type lock (m_mutex);
        area_map_t::iterator area (m_areas.find(mem_id));
        if (area == m_areas.end())
        {
          throw std::runtime_error ( "no such memory: "
                                   + boost::lexical_cast<std::string>(mem_id)
                                   );
        }

        if (pid)
        {
          area->second->detach_process (pid);

          if (!area->second->in_use())
          {
            area_map_t::iterator area_it (area);

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
          }
        }
      }
        if (m_areas.find(mem_id) != m_areas.end())
        {
          try
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
          }
          catch (...)
          {
            // unroll
      {
        lock_type lock (m_mutex);
        area_map_t::iterator area (m_areas.find(mem_id));
        if (area == m_areas.end())
        {
          throw std::runtime_error ( "no such memory: "
                                   + boost::lexical_cast<std::string>(mem_id)
                                   );
        }

        if (pid)
        {
          area->second->attach_process (pid);
        }
      }
            throw;
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
          auto const mem_id (segments.front());
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

          if (!area->second->in_use())
          {
            area_map_t::iterator area_it (area);

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
          }
        }
      }
          segments.pop_front();
        }
      }

      void
      manager_t::add_area (manager_t::area_ptr const &area)
      {
        lock_type lock (m_mutex);

        if (m_areas.find (area->get_id ()) != m_areas.end())
        {
          throw std::runtime_error
            ("cannot add another gpi segment: id already in use!");
        }

        m_areas [area->get_id ()] = area;
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

      void
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

        gpi::pc::type::handle_t hdl (area->alloc (proc_id, size, name, flags));

        add_handle (hdl, seg_id);

        return hdl;
      }

      void
      manager_t::remote_free (const gpi::pc::type::handle_t hdl)
      {
        area_ptr area (get_area_by_handle (hdl));

        area->remote_free (hdl);
        del_handle (hdl);
      }

      void
      manager_t::free (const gpi::pc::type::handle_t hdl)
      {
        area_ptr area (get_area_by_handle (hdl));

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

          for (gpi::rank_t rank = 0; rank < _gaspi_context.number_of_nodes(); ++rank)
          {
            costs[_gaspi_context.hostname_of_rank (rank)] += area->get_transfer_costs (transfer, rank);
          }
        }

        return costs;
      }

      type::memcpy_id_t manager_t::memcpy
        ( type::memory_location_t const & dst
        , type::memory_location_t const & src
        , const type::size_t amount
        )
      {
        auto& src_area (*get_area_by_handle (src.handle));
        auto& dst_area (*get_area_by_handle (dst.handle));

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
      manager_t::remote_add_memory ( const gpi::pc::type::segment_id_t seg_id
                                   , iml::segment_description const& description
                                   , unsigned long total_size
                                   , global::topology_t& topology
                                   )
      {
        area_ptr_t area (create_area (description, total_size, topology, _handle_generator, _gaspi_context, 0));
        area->set_id (seg_id);
        add_area (area);
      }

      namespace
      {
        //! \note for beegfs, master creates file that slaves need to
        //! open determining filesize from the opened file. thus, to
        //! avoid a race, the master is doing this before all
        //! others. gaspi on the other side needs simultaneous
        //! initialization for gaspi_segment_create etc.
        bool require_earlier_master_initialization
          (iml::segment_description const& description)
        {
          return fhg::util::visit<bool>
            ( description
            , [&] (iml::beegfs_segment_description const&)
              {
                return true;
              }
            , [&] (iml::gaspi_segment_description const&)
              {
                return false;
              }
            );
        }
      }

      gpi::pc::type::segment_id_t
      manager_t::add_memory ( const gpi::pc::type::process_id_t proc_id
                            , iml::segment_description const& description
                            , unsigned long total_size
                            , global::topology_t& topology
                            )
      {

        type::segment_id_t const id
          (_handle_generator.next (gpi::pc::type::segment::SEG_INVAL));
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
          area_ptr_t area = create_area (description, total_size, topology, _handle_generator, _gaspi_context, proc_id);
          area->set_id (id);

          add_area (area);

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
                                                  , proc_id
                                                  )
                                    );
                    area->set_id (id);
                    add_area (area);
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
      manager_t::remote_del_memory ( const gpi::pc::type::segment_id_t seg_id
                                   , global::topology_t& topology
                                   )
      {
        del_memory (0, seg_id, topology);
      }

      void
      manager_t::del_memory ( const gpi::pc::type::process_id_t proc_id
                            , const gpi::pc::type::segment_id_t seg_id
                            , global::topology_t& topology
                            )
      {
        if (0 == seg_id)
          throw std::runtime_error ("invalid segment id");

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

          if (proc_id > 0)
          {
            topology.del_memory (seg_id);
          }
        }
      }
    }
  }
}
