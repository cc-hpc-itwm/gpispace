#include "gpi_area.hpp"

#include <fhglog/minimal.hpp>
#include <gpi-space/gpi/api.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      gpi_area_t::gpi_area_t ( const gpi::pc::type::id_t id
                             , const gpi::pc::type::process_id_t creator
                             , const std::string & name
                             , const gpi::pc::type::size_t size
                             , const gpi::pc::type::flags_t flags
                             , void * dma_ptr
                             )
          : area_t ( gpi::pc::type::segment::SEG_GPI
                   , id
                   , creator
                   , name
                   , size
                   , flags
                   )
          , m_ptr (dma_ptr)
      {
        // total memory size is required for boundary checks
        m_total_memsize = gpi::api::gpi_api_t::get().number_of_nodes () * size;
      }

      gpi_area_t::~gpi_area_t ()
      { }

      area_t::grow_direction_t
      gpi_area_t::grow_direction (const gpi::pc::type::flags_t flgs) const
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

      bool
      gpi_area_t::is_allowed_to_attach (const gpi::pc::type::process_id_t) const
      {
        return false;
      }

      int gpi_area_t::get_type_id () const
      {
        return area_type;
      }

      void
      gpi_area_t::check_bounds ( const gpi::pc::type::handle::descriptor_t &hdl
                               , const gpi::pc::type::offset_t start
                               , const gpi::pc::type::offset_t end
                               ) const
      {
        if (gpi::flag::is_set (hdl.flags, gpi::pc::type::handle::F_GLOBAL))
        {
          // handle size is actually #nodes*hdl.size
          const gpi::pc::type::size_t global_size
              ( gpi::api::gpi_api_t::get().number_of_nodes ()
              * hdl.size
              );
          if (! (start < global_size && end < global_size))
          {
            throw std::invalid_argument
                ("out-of-bounds: access to global handle outside boundaries");
          }
        }
        else
        {
          if (! (start < hdl.size && end < hdl.size))
          {
            CLOG( ERROR
               , "gpi.memory"
               , "out-of-bounds access:"
               << " hdl=" << hdl
               << " size=" << hdl.size
               << " range=["<<start << ", " << end << "]"
               );
            throw std::invalid_argument
                ("out-of-bounds: access to local handle outside boundaries");
          }
        }
      }

      void
      gpi_area_t::alloc_hook (const gpi::pc::type::handle::descriptor_t &hdl)
      {
        if (gpi::flag::is_set (hdl.flags, gpi::pc::type::handle::F_GLOBAL))
        {
          // TODO (ap):
          // gpi_space_com_api::global_alloc (desc.hdl, desc.size);
          //     make sure to release locks!
          LOG(ERROR, "global GPI allocations are not yet fully implemented");
        }
      }

      void
      gpi_area_t::free_hook (const gpi::pc::type::handle::descriptor_t &hdl)
      {
        if (gpi::flag::is_set (hdl.flags, gpi::pc::type::handle::F_GLOBAL))
        {
          // TODO (ap):
          // gpi_space_com_api::global_free (desc.hdl);
          //     make sure to release locks!
          LOG(ERROR, "global GPI deallocations are not yet fully implemented");
        }
      }
    }
  }
}
