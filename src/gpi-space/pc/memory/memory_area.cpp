#include <gpi-space/pc/memory/memory_area.hpp>

#include <stack>

#include <fhglog/LogMacros.hpp>
#include <fhg/assert.hpp>

#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>

#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/type/handle.hpp>

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
                     , const gpi::pc::type::flags_t flags
                     , handle_generator_t& handle_generator
                     )
        : m_descriptor ( GPI_PC_INVAL
                       , type
                       , creator
                       , name
                       , size
                       , flags
                       )
        , m_mmgr (NULL)
        , _handle_generator (handle_generator)
      {
        reinit ();
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

      void area_t::init () {}

      void area_t::reinit ()
      {
        dtmmgr_finalize (&m_mmgr);
        dtmmgr_init (&m_mmgr, m_descriptor.local_size, 1);

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

      void area_t::set_owner (gpi::pc::type::id_t o)
      {
        m_descriptor.creator = o;
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
                                       , gpi::pc::F_PERSISTENT
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
          this->free (hdl);
        }
      }

      bool area_t::in_use () const
      {
        lock_type lock (m_mutex);
        return m_descriptor.nref > 0;
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

      gpi::pc::type::flags_t
      area_t::flags () const
      {
        return descriptor ().flags;
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

      bool area_t::is_eligible_for_deletion () const
      {
        lock_type lock (m_mutex);
        assert (m_descriptor.nref == m_attached_processes.size());
        if (m_descriptor.nref)
        {
          return false;
        }
        else if (gpi::flag::is_set ( m_descriptor.flags
                                   , gpi::pc::F_PERSISTENT
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
        if (! (loc.offset < hdl_it->second.size && (loc.offset + size) <= hdl_it->second.size))
        {
          CLOG( ERROR
              , "shm.memory"
              , "out-of-bounds access:"
              << " hdl=" << hdl_it->second
              << " size=" << hdl_it->second.size
              << " range=["<<loc.offset << ", " << loc.offset+size << "]"
              );
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

      int
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
        hdl.creator = GPI_PC_INVAL;
        hdl.flags = gpi::pc::F_GLOBAL | gpi::pc::F_PERSISTENT;

        internal_alloc (hdl);

        if (hdl.offset != offset)
        {
          LOG ( ERROR
              , "remote_alloc failed: expected-offset = " << offset
              << " actual-offset = " << hdl.offset
              );
          dtmmgr_free ( &m_mmgr
                      , hdl.id
                      , grow_direction (hdl.flags)
                      );
          throw std::runtime_error("offset mismatch");
        }
        else
        {
          update_descriptor_from_mmgr ();
          m_handles [hdl.id] = hdl;
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
        m_descriptor.avail = dtmmgr_memfree (m_mmgr);
        m_descriptor.allocs =
            dtmmgr_numhandle (m_mmgr, ARENA_UP)
          + dtmmgr_numhandle (m_mmgr, ARENA_DOWN);
        // dtmmgr_numalloc -> total allocs
        // dtmmgr_numfree -> total frees
        m_descriptor.ts.touch();
      }

      void area_t::internal_alloc (gpi::pc::type::handle::descriptor_t &hdl)
      {
        if (m_descriptor.avail < hdl.local_size)
        {
          LOG( ERROR
             , "not enough memory:"
             << " total size = " << hdl.size
             << " local size = " << hdl.local_size
             << " segment = " << m_descriptor.id
             << " avail = " << m_descriptor.avail
             );
          throw std::runtime_error ("out of memory");
        }

        Arena_t arena = grow_direction(hdl.flags);

        AllocReturn_t alloc_return
            (dtmmgr_alloc (&m_mmgr, hdl.id, arena, hdl.local_size));

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
          }
          break;
        case ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY:
          LOG( WARN
             , "not enough contiguous memory available:"
             << " requested_size = " << hdl.local_size
             << " segment = " << m_descriptor.id
             << " avail = " << m_descriptor.avail
             );
          // TODO:
          //    defrag (local_size);
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
             << " requested_size=" << hdl.local_size
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
             << " requested_size = " << hdl.local_size
             << " handle = " << hdl.id
             << " segment = " << m_descriptor.id
             );
          throw std::runtime_error ("allocation failed");
          break;
        default:
          LOG( ERROR
             ,  "unexpected error during allocation:"
             << " requested_size = " << hdl.local_size
             << " handle = " << hdl.id
             << " segment = " << m_descriptor.id
             << " error = " << alloc_return
             );
          throw std::runtime_error ("unexpected return code");
          break;
        }
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

        Arena_t arena (grow_direction(desc.flags));
        HandleReturn_t handle_return (dtmmgr_free ( &m_mmgr
                                                  , hdl
                                                  , arena
                                                  )
                                     );
        switch (handle_return)
        {
        case RET_SUCCESS:
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

        Arena_t arena (grow_direction(desc.flags));
        HandleReturn_t handle_return (dtmmgr_free ( &m_mmgr
                                                  , hdl
                                                  , arena
                                                  )
                                     );
        switch (handle_return)
        {
        case RET_SUCCESS:
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
          LOG(ERROR, "cannot find descriptor for handle " << hdl);
          throw std::runtime_error ("no such handle");
        }
      }

      void area_t::list_allocations ( const gpi::pc::type::process_id_t proc
                                    , gpi::pc::type::handle::list_t & list
                                    ) const
      {
        lock_type lock (m_mutex);

        for ( handle_descriptor_map_t::const_iterator pos (m_handles.begin())
            ; pos != m_handles.end()
            ; ++pos
            )
        {
          if (  (pos->second.flags & gpi::pc::F_EXCLUSIVE)
             && (pos->second.creator != proc)
             )
          {
            continue;
          }

          list.push_back (pos->second);
        }
      }

      bool
      area_t::is_allowed_to_attach (const gpi::pc::type::process_id_t proc) const
      {
        return (!gpi::flag::is_set (descriptor ().flags, gpi::pc::F_EXCLUSIVE))
            || (proc == descriptor ().creator);
      }

      gpi::pc::type::size_t
      area_t::attach_process (const gpi::pc::type::process_id_t id)
      {
        lock_type lock (m_mutex);
        if (is_allowed_to_attach (id))
        {
          if (m_attached_processes.insert (id).second)
          {
            ++m_descriptor.nref;
          }
          return m_descriptor.nref;
        }
        else
        {
          throw std::runtime_error
            ("permission denied, exclusive segment and you are not the owner");
        }
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

      namespace detail
      {
        struct writer
        {
          writer ( area_t & a
                 , gpi::pc::type::memory_location_t loc
                 , const void *buffer
                 , gpi::pc::type::size_t amount
                 )
            : m_area (a)
            , m_location (loc)
            , m_buffer (buffer)
            , m_amount (amount)
          {}

          void operator () ()
          {
            m_area.write_to (m_location, m_buffer, m_amount);
          }
        private:
          area_t & m_area;
          gpi::pc::type::memory_location_t m_location;
          const void * m_buffer;
          gpi::pc::type::size_t m_amount;
        };

        struct reader
        {
          reader ( area_t & a
                 , gpi::pc::type::memory_location_t loc
                 , void *buffer
                 , gpi::pc::type::size_t amount
                 )
            : m_area (a)
            , m_location (loc)
            , m_buffer (buffer)
            , m_amount (amount)
          {}

          void operator () ()
          {
            m_area.read_from (m_location, m_buffer, m_amount);
          }
        private:
          area_t & m_area;
          gpi::pc::type::memory_location_t m_location;
          void * m_buffer;
          gpi::pc::type::size_t m_amount;
        };

        struct copy
        {
          copy ( area_t & src
               , area_t & dst
               , gpi::pc::type::memory_location_t src_loc
               , gpi::pc::type::memory_location_t dst_loc
               , gpi::pc::type::size_t amount
               , area_t::memory_pool_t & buffer_pool
               )
            : m_src (src)
            , m_dst (dst)
            , m_src_loc (src_loc)
            , m_dst_loc (dst_loc)
            , m_amount (amount)
            , m_pool (buffer_pool)
          {}

          void operator () ()
          {
            std::auto_ptr<buffer_t> buffer (m_pool.get().release());

            size_t remaining = m_amount;

            while (remaining)
            {
              const size_t to_read = std::min (remaining, buffer->size ());

              const size_t num_read = m_src.read_from ( m_src_loc
                                                      , buffer->data ()
                                                      , to_read
                                                      );
              if (0 == num_read)
              {
                MLOG ( ERROR
                     , "could not read " << buffer->size () << " bytes"
                     << " from " << m_src_loc
                     << " remaining " << remaining
                     );
                m_pool.put (buffer);
                throw std::runtime_error ("could not read");
              }

              buffer->used (num_read);

              const size_t num_written = m_dst.write_to ( m_dst_loc
                                                        , buffer->data ()
                                                        , buffer->used ()
                                                        );

              assert (num_read == num_written);

              buffer->used (num_read - num_written);

              remaining -= num_read;
            }

            m_pool.put (buffer);
          }
        private:
          area_t & m_src;
          area_t & m_dst;
          gpi::pc::type::memory_location_t m_src_loc;
          gpi::pc::type::memory_location_t m_dst_loc;
          gpi::pc::type::size_t m_amount;
          area_t::memory_pool_t & m_pool;
        };
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

      int
      area_t::get_send_tasks ( area_t &
                             , const gpi::pc::type::memory_location_t
                             , const gpi::pc::type::memory_location_t
                             , gpi::pc::type::size_t
                             , gpi::pc::type::size_t
                             , task_list_t &
                             )
      {
        throw std::runtime_error ("get_send_tasks() not implemented");
      }

      int
      area_t::get_recv_tasks ( area_t &
                             , const gpi::pc::type::memory_location_t
                             , const gpi::pc::type::memory_location_t
                             , gpi::pc::type::size_t
                             , gpi::pc::type::size_t
                             , task_list_t &
                             )
      {
        throw std::runtime_error ("get_recv_tasks() not implemented");
      }

      int
      area_t::get_transfer_tasks ( const gpi::pc::type::memory_location_t src
                                 , const gpi::pc::type::memory_location_t dst
                                 , area_t & dst_area
                                 , gpi::pc::type::size_t amount
                                 , gpi::pc::type::size_t queue
                                 , memory_pool_t & buffer_pool
                                 , task_list_t & tasks
                                 )
      {
        const bool src_is_local
          (         is_local (gpi::pc::type::memory_region_t( src
                                                            , amount
                                                            )
                             )
          );
        const bool dst_is_local
          (dst_area.is_local (gpi::pc::type::memory_region_t( dst
                                                            , amount
                                                            )
                             )
          );

        // vertical copy (memcpy/read/write)
        if (src_is_local && dst_is_local)
        {
          void *src_ptr = pointer_to (src);
          void *dst_ptr = dst_area.pointer_to (dst);

          if (src_ptr)
          {
            tasks.push_back
              (boost::make_shared<task_t>
              ( "write_to: "
              + boost::lexical_cast<std::string> (dst)
              + " <- "
              + boost::lexical_cast<std::string> (src)
              + " "
              + boost::lexical_cast<std::string> (amount)

              , detail::writer ( dst_area
                               , dst
                               , src_ptr
                               , amount
                               )
              ));
            return 0;
          }
          else if (dst_ptr)
          {
            tasks.push_back
              (boost::make_shared<task_t>
              ( "read_from: "
              + boost::lexical_cast<std::string> (dst)
              + " <- "
              + boost::lexical_cast<std::string> (src)
              + " "
              + boost::lexical_cast<std::string> (amount)

              , detail::reader ( *this
                               , src
                               , dst_ptr
                               , amount
                               )
              ));
            return 0;
          }
          else
          {
            tasks.push_back
              (boost::make_shared<task_t>
              ( "copy: "
              + boost::lexical_cast<std::string> (dst)
              + " <- "
              + boost::lexical_cast<std::string> (src)
              + " "
              + boost::lexical_cast<std::string> (amount)

              , detail::copy ( *this
                             , dst_area
                             , src
                             , dst
                             , amount
                             , buffer_pool
                             )
              ));
            return 0;
          }
        }

        // only reached when:
        //    - non-local segments
        //    - no raw memory available

        // horizontal copy (same type)
        if (type () == dst_area.type ())
        {
          return get_specific_transfer_tasks ( src
                                             , dst
                                             , dst_area
                                             , amount
                                             , queue
                                             , tasks
                                             );
        }
        // diagonal copy (non-local different types)
        else
        {
          if (src_is_local)
          {
            // send from local source to remote destination
            return dst_area.get_send_tasks ( *this
                                           , src
                                           , dst
                                           , amount
                                           , queue
                                           , tasks
                                           );
          }
          else if (dst_is_local)
          {
            // receive from remote src to local destination
            return this->get_recv_tasks ( dst_area
                                        , dst
                                        , src
                                        , amount
                                        , queue
                                        , tasks
                                        );
          }
          else
          {
            MLOG ( ERROR
                 , "unsupported memory transfer: both regions are remote:"
                 << " src := [" << src << "]"
                 << " dst := [" << dst << "]"
                 );
            throw std::runtime_error
              ("unsupported memory transfer: both regions are remote");
          }
        }

        return 0;
      }
    }
  }
}