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
        // make sure that there are no remaining
        // accesses to segments queued

        //     i.e. cancel/remove all items in the memory transfer component
      }

      gpi::pc::type::segment_id_t
      manager_t::register_segment ( const gpi::pc::type::process_id_t creator
                                  , std::string const & name
                                  , const gpi::pc::type::size_t sz
                                  , const gpi::pc::type::flags_t flags
                                  )
      {
        LOG(TRACE, "registering new shared memory segment: " << name << " with size " << sz << " via process " << creator);

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

          // TODO: update timestamps

          m_segments [seg->id()] = seg;

          return seg->id ();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not open shared memory segment: " << ex.what());
          throw;
        }
      }

      gpi::pc::type::segment::list_t manager_t::get_listing () const
      {
        lock_type lock (m_mutex);

        gpi::pc::type::segment::list_t list;

        for ( segment_map_t::const_iterator seg (m_segments.begin())
            ; seg != m_segments.end()
            ; ++seg
            )
        {
          list.push_back (seg->second->descriptor());
        }

        return list;
      }
    }
  }
}
