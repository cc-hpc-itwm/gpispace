#include "manager.hpp"

#include <fhglog/minimal.hpp>

namespace gpi
{
  namespace pc
  {
    namespace segment
    {
      manager_t::manager_t ()
        : m_segment_id (gpi::pc::type::segment::SEG_SHARED)
      {}

      manager_t::~manager_t ()
      {
        try
        {
          clear ();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not clear segment manager: " << ex.what());
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
        while (! m_segments.empty())
        {
          unregister_segment (m_segments.begin()->first);
        }
      }

      manager_t::segment_ptr
      manager_t::get_segment (const gpi::pc::type::segment_id_t seg_id)
      {
        lock_type lock (m_mutex);
        return m_segments.at (seg_id);
      }

      void manager_t::add_special_segment ( std::string const & name
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

        segment_ptr seg (new gpi::pc::segment::segment_t (desc, ptr));

        lock_type lock (m_mutex);
        assert (m_segments.find (id) == m_segments.end());
        m_segments [seg->id()] = seg;

        // move counter if required
        if (m_segment_id <= id)
          m_segment_id = id + 1;

        LOG(TRACE, "special memory segment registered: " << seg->name() << " (" << seg->id() << ") size " << seg->size() << " @" << seg->ptr());

        segment_added (id);
      }

      gpi::pc::type::segment_id_t
      manager_t::register_segment ( const gpi::pc::type::process_id_t creator
                                  , std::string const & name
                                  , const gpi::pc::type::size_t sz
                                  , const gpi::pc::type::flags_t flags
                                  )
      {
        try
        {
          segment_ptr seg (new gpi::pc::segment::segment_t (name, sz));

          seg->open ();

          lock_type lock (m_mutex);

          if (gpi::pc::type::segment::SEG_INVAL == m_segment_id) // very unlikely
          {
            LOG(ERROR, "overflow of segment ids occured!");
            throw std::runtime_error ("cannot attach more segments, id overflow");
          }

          seg->assign_id (m_segment_id);
          ++m_segment_id;

          seg->descriptor().creator = creator;
          seg->descriptor().flags = flags;

          // important: if F_EXCLUSIVE is set, F_NOUNLINK does not make any sense
          if (seg->descriptor().flags & gpi::pc::type::segment::F_EXCLUSIVE)
          {
            seg->unlink();
          }

          m_segments [seg->id()] = seg;

          LOG(TRACE, "shared memory segment registered: " << seg->name() << " (" << seg->id() << ") size " << seg->size() << " @" << seg->ptr());

          segment_added (seg->id());
          return seg->id ();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not open shared memory segment: " << ex.what());
          throw;
        }
      }

      void manager_t::unregister_segment(const gpi::pc::type::segment_id_t seg_id)
      {
        lock_type lock (m_mutex);

        if (m_segments.find (seg_id) == m_segments.end())
          return;

        segment_ptr seg (m_segments.at (seg_id));
        if (seg->descriptor().nref)
        {
          throw std::runtime_error ("segment is still inuse, cannot unregister");
        }

        if (!gpi::flag::is_set ( seg->descriptor().flags
                               , gpi::pc::type::segment::F_NOUNLINK | gpi::pc::type::segment::F_EXCLUSIVE
                               )
           )
        {
          seg->unlink();
        }

        m_segments.erase (seg_id);

        LOG(TRACE, "memory segment unregistered: " << seg->name() << " (" << seg_id << ")");

        segment_removed (seg_id);
      }

      void manager_t::get_listing (gpi::pc::type::segment::list_t & l) const
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

      gpi::pc::type::size_t manager_t::increment_refcount (const gpi::pc::type::segment_id_t seg_id)
      {
        lock_type lock (m_mutex);
        segment_ptr seg (m_segments.at (seg_id));
        return ++(seg->descriptor().nref);
      }

      gpi::pc::type::size_t manager_t::decrement_refcount (const gpi::pc::type::segment_id_t seg_id)
      {
        lock_type lock (m_mutex);
        segment_ptr seg (m_segments.at (seg_id));

        assert (seg->descriptor().nref > 0);
        --(seg->descriptor().nref);

        if (0 == seg->descriptor().nref)
        {
          if (0 == (seg->descriptor().flags & gpi::pc::type::segment::F_PERSISTENT))
          {
            unregister_segment (seg_id);
          }
          else if (seg->descriptor().flags & gpi::pc::type::segment::F_EXCLUSIVE)
          {
            unregister_segment (seg_id);
          }
        }

        return seg->descriptor().nref;
      }
    }
  }
}
