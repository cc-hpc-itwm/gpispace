#include "memory_area.hpp"

#include <fhglog/minimal.hpp>

#include <gpi-space/pc/type/handle.hpp>
#include <gpi-space/pc/memory/handle_generator.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      /***************************************************/
      /*                   area_t                        */
      /***************************************************/

      area_t::area_t (const segment_ptr &segment)
        : m_segment (segment)
        , m_mmgr (NULL)
      {
        assert (m_segment->size() > 0);

        dtmmgr_init (&m_mmgr, m_segment->size(), 1);
      }

      area_t::~area_t ()
      {
        dtmmgr_finalize (&m_mmgr);
      }

      void area_t::alloc ( const gpi::pc::type::size_t size
                         , const std::string & name
                         , const gpi::pc::type::flags_t flags
                         )
      {
        lock_type lock (m_mutex);

        gpi::pc::type::handle::descriptor_t desc;
        desc.id = handle_generator_t::get().next (m_segment->id());
        desc.size = size;
        desc.name = name;
        desc.flags = flags;

        Arena_t arena;
        if (gpi::flag::is_set ( flags
                              , gpi::pc::type::handle::F_GLOBAL
                              )
           )
        {
          arena = ARENA_GLOBAL;
        }
        else
        {
          arena = ARENA_LOCAL;
        }

        AllocReturn_t alloc_return (dtmmgr_alloc (&m_mmgr, desc.id, arena, size));

        DLOG(TRACE, "ALLOC: handle = " << id << " arena = " << arena << " size = " << size << " return = " << alloc_return);

        switch (alloc_return)
        {
        case ALLOC_SUCCESS:
          {
            tmmgr_offset_size (&m_mmgr, desc.id, &desc.offset, NULL);
            m_segment->descriptor().avail -= size;
            m_segment->descriptor().allocs += 1;
            m_segment->descriptor().ts.touch (gpi::pc::type::time_stamp_t::TOUCH_ACCESSED);
            m_handles [desc.id] = desc;
          }
          break;
        case ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY:
          LOG(WARN, "not enough contiguous memory available: requested_size = " << size << " segment = " << m_segment->id() << " avail = " << m_segment->descriptor().avail);
          throw std::runtime_error ("not enough contiguous memory available");
          break;
        case ALLOC_DUPLICATE_HANDLE:
          LOG(ERROR, "duplicate: handle = " << desc.id << " segment = " << m_segment->id());
          throw std::runtime_error ("duplicate handle");
          break;
        case ALLOC_INSUFFICIENT_MEMORY:
          LOG(ERROR, "not enough memory: requested_size = " << size << " segment = " << m_segment->id() << " avail = " << m_segment->descriptor().avail);
          throw std::runtime_error ("out of memory");
          break;
        case ALLOC_FAILURE:
          LOG(ERROR, "internal error during allocation: requested_size = " << size << " segment = " << m_segment->id());
          throw std::runtime_error ("allocation failed");
          break;
        default:
          LOG(FATAL, "unexpected error during allocation: requested_size = " << size << " segment = " << m_segment->id());
          throw std::runtime_error ("unexpected return code");
          break;
        }
      }

      void area_t::free (const gpi::pc::type::handle_id_t hdl)
      {
        lock_type lock (m_mutex);
        if (m_handles.find(hdl) == m_handles.end())
        {
          LOG(WARN, "possible double free detected: " << gpi::pc::type::handle_t(hdl));
          throw std::runtime_error ("no such handle");
        }

        const gpi::pc::type::handle::descriptor_t desc (m_handles.at(hdl));
        m_handles.erase (hdl);

        Arena_t arena;
        if ( gpi::flag::is_set ( desc.flags
                               , gpi::pc::type::handle::F_GLOBAL
                               )
           )
        {
          arena = ARENA_GLOBAL;
        }
        else
        {
          arena = ARENA_LOCAL;
        }

        HandleReturn_t handle_return (dtmmgr_free ( &m_mmgr
                                                  , hdl
                                                  , arena
                                                  )
                                     );
        switch (handle_return)
        {
        case RET_SUCCESS:
          DLOG(TRACE, "handle free'd: " << desc);
          m_segment->descriptor().avail += desc.size;
          m_segment->descriptor().allocs -= 1;
          m_segment->descriptor().ts.touch (gpi::pc::type::time_stamp_t::TOUCH_ACCESSED);
          break;
        case RET_HANDLE_UNKNOWN:
          LOG(ERROR, "inconsistency detected: mmgr did not know handle " << desc);
          throw std::runtime_error ("no such handle");
          break;
        case RET_FAILURE:
          LOG(ERROR, "unknown error during free: handle = " << desc);
          throw std::runtime_error ("no such handle");
          break;
        }
      }

      gpi::pc::type::size_t area_t::used_mem_size () const
      {
        return total_mem_size() - free_mem_size();
      }

      gpi::pc::type::size_t area_t::total_mem_size () const
      {
        lock_type lock (m_mutex);
        return m_segment->size();
      }

      gpi::pc::type::size_t area_t::free_mem_size () const
      {
        lock_type lock (m_mutex);
        return m_segment->descriptor().avail;
      }
    }
  }
}
