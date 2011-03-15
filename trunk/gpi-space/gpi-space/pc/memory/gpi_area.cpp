#include "gpi_area.hpp"

#include <utility>

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
        m_min_local_offset = gpi::api::gpi_api_t::get().rank() * size;
        m_max_local_offset = m_min_local_offset + size - 1;

        CLOG( TRACE
            , "gpi.memory"
            , "GPI memory created:"
            <<" per-node: " << size
            <<" total: " << m_total_memsize
            <<" range:"
            <<" ["
            << m_min_local_offset << "," << m_max_local_offset
            << "]"
            );
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

      struct dma_transfer_t
      {
        dma_transfer_t () {}
        dma_transfer_t ( const gpi::pc::type::offset_t rem_off
                       , const gpi::pc::type::size_t   rnk
                       , const gpi::pc::type::offset_t loc_off
                       , const gpi::pc::type::size_t   amt
                       )
          : remote_offset(rem_off)
          , rank (rnk)
          , local_offset(loc_off)
          , amount(amt)
        {}

        gpi::pc::type::offset_t remote_offset;
        gpi::pc::type::size_t   rank;
        gpi::pc::type::offset_t local_offset;
        gpi::pc::type::size_t   amount;
      };

      void
      gpi_area_t::write_dma ( const gpi::pc::type::memory_location_t & dst
                            , const gpi::pc::type::memory_location_t & src
                            , const gpi::pc::type::size_t size
                            , const gpi::pc::type::queue_id_t queue
                            )
      {
      }

      void
      gpi_area_t::read_dma ( const gpi::pc::type::memory_location_t & src
                           , const gpi::pc::type::memory_location_t & dst
                           , const gpi::pc::type::size_t size
                           , const gpi::pc::type::queue_id_t queue
                           )
      {
        using namespace gpi::pc::type;

        handle::descriptor_t src_hdl_desc (descriptor(src.handle));
        handle::descriptor_t dst_hdl_desc (descriptor(dst.handle));

        typedef std::list<dma_transfer_t> dma_list_t;
        dma_list_t to_read;

        offset_t local_offset (dst_hdl_desc.offset + dst.offset);

        if ((src.offset + size) <= src_hdl_desc.size)
        {
        }

        // calculate src nodes
        //       (rank, offset, amount)


        gpi::api::gpi_api_t & api (gpi::api::gpi_api_t::get());

        if (api.max_dma_requests_reached (queue))
        {
          api.wait_dma(queue);
        }

        /*
        api.read_dma ( const offset_t local_offset
                     , const offset_t remote_offset
                     , const size_t amount
                     , const rank_t from_node
                     , t.queue
                     );
        */
      }

      void *
      gpi_area_t::ptr ()
      {
        return m_ptr;
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
            CLOG( ERROR
               , "gpi.memory"
               , "out-of-bounds access:"
               << " hdl=" << hdl
               << " size=" << hdl.size
               << " globalsize=" << global_size
               << " range=["<<start << ", " << end << "]"
               );
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

      gpi::pc::type::offset_t
      gpi_area_t::handle_to_global_offset ( const gpi::pc::type::offset_t hdl_offset
                                          , const gpi::pc::type::size_t per_node_size
                                          ) const
      {
        gpi::pc::type::offset_t global_offset (0);
        gpi::pc::type::offset_t tmp (hdl_offset);
        while (tmp >= per_node_size)
        {
          tmp -= per_node_size;
          global_offset += this->size();
        }
        return global_offset + tmp;
      }

      typedef std::pair<gpi::pc::type::id_t, gpi::pc::type::id_t> rank_range_t;
      static
      rank_range_t
      handle_subscript_to_nodes ( const gpi::pc::type::handle::descriptor_t &hdl
                                , const gpi::pc::type::offset_t begin
                                , const gpi::pc::type::offset_t end
                                )
      {
        const gpi::pc::type::id_t
            slice_start_rank (begin / hdl.size);
        const gpi::pc::type::id_t
            slice_end_rank ( (end-1) / hdl.size);
        return rank_range_t (slice_start_rank, slice_end_rank);
      }

      bool
      gpi_area_t::is_range_local( const gpi::pc::type::handle::descriptor_t &hdl
                                , const gpi::pc::type::offset_t begin
                                , const gpi::pc::type::offset_t end
                                ) const
      {
        LOG(TRACE, "checking range: " << "hdl=" << hdl << " begin=" << begin << " end=" << end);
        if (gpi::flag::is_set (hdl.flags, gpi::pc::type::handle::F_GLOBAL))
        {
          rank_range_t ranks (handle_subscript_to_nodes (hdl, begin, end));
          const gpi::pc::type::id_t my_rank (gpi::api::gpi_api_t::get().rank());
          return (ranks.first  == my_rank)
              && (ranks.second == my_rank);
        }
        else
        {
          return ((hdl.offset + begin) < size())
              && ((hdl.offset + end)   < size());
        }
      }
    }
  }
}
