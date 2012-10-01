#include "sfs_area.hpp"

#include <fhglog/minimal.hpp>
#include <boost/make_shared.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      sfs_area_t::sfs_area_t ( const gpi::pc::type::process_id_t creator
                             , const std::string & path
                             , const gpi::pc::type::size_t size        // total
                             , const gpi::pc::type::flags_t flags
                             , const std::string & meta_data
                             , gpi::pc::global::itopology_t & topology
                             )
        : area_t ( sfs_area_t::area_type
                 , creator
                 , path
                 , size
                 , flags
                 )
        , m_ptr (0)
        , m_path (path)
        , m_meta (meta_data)
        , m_size (size)
        , m_min_local_offset (0)
        , m_max_local_offset (size)
        , m_topology (topology)
      {
        CLOG( TRACE
            , "gpi.memory"
            , "SFS memory created:"
            <<" size: " << size
            <<" range:"
            <<" ["
            << m_min_local_offset << "," << m_max_local_offset
            << "]"
            << " path: " << m_path
            );
      }

      sfs_area_t::~sfs_area_t ()
      {}

      area_t::grow_direction_t
      sfs_area_t::grow_direction (const gpi::pc::type::flags_t flgs) const
      {
        if (gpi::flag::is_set (flgs, gpi::pc::type::handle::F_GLOBAL))
        {
          return GROW_UP;
        }
        else
        {
          return GROW_DOWN;
        }
      }

      void *
      sfs_area_t::ptr ()
      {
        return m_ptr;
      }

      bool
      sfs_area_t::is_allowed_to_attach (const gpi::pc::type::process_id_t) const
      {
        return false;
      }

      void
      sfs_area_t::check_bounds ( const gpi::pc::type::handle::descriptor_t &hdl
                               , const gpi::pc::type::offset_t start
                               , const gpi::pc::type::size_t   amount
                               ) const
      {
        if (! (start < hdl.size && (start + amount) <= hdl.size))
        {
          CLOG( ERROR
              , "gpi.memory"
              , "out-of-bounds access:"
              << " hdl=" << hdl
              << " size=" << hdl.size
              << " range=["<<start << ", " << start + amount << "]"
              );
          throw std::invalid_argument
            ("out-of-bounds: access to handle outside boundaries");
        }
      }

      void
      sfs_area_t::alloc_hook (const gpi::pc::type::handle::descriptor_t &hdl)
      {
        if (  gpi::flag::is_set (hdl.flags, gpi::pc::type::handle::F_GLOBAL)
           && hdl.creator != (gpi::pc::type::process_id_t)(-1)
           )
        {
          m_topology.alloc ( descriptor ().id
                           , hdl.id
                           , hdl.offset
                           , hdl.size
                           , hdl.local_size
                           , hdl.name
                           );
        }
      }

      void
      sfs_area_t::free_hook (const gpi::pc::type::handle::descriptor_t &hdl)
      {
        if (gpi::flag::is_set (hdl.flags, gpi::pc::type::handle::F_GLOBAL))
        {
          m_topology.free(hdl.id);
        }
      }

      bool
      sfs_area_t::is_range_local( const gpi::pc::type::handle::descriptor_t &hdl
                                , const gpi::pc::type::offset_t begin
                                , const gpi::pc::type::offset_t end
                                ) const
      {
        return true;
      }

      gpi::pc::type::size_t
      sfs_area_t::get_local_size ( const gpi::pc::type::size_t size
                                 , const gpi::pc::type::flags_t flgs
                                 ) const
      {
        return size;
      }

      int
      sfs_area_t::get_specific_transfer_tasks ( const gpi::pc::type::memory_location_t src
                                              , const gpi::pc::type::memory_location_t dst
                                              , area_t & dst_area
                                              , gpi::pc::type::size_t amount
                                              , gpi::pc::type::size_t queue
                                              , task_list_t & tasks
                                              )
      {
        throw std::runtime_error ("not yet implemented");
      }
    }
  }
}
