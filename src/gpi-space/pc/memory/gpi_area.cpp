#include <gpi-space/pc/memory/gpi_area.hpp>

#include <utility>

#include <fhg/assert.hpp>
#include <fhglog/LogMacros.hpp>
#include <gpi-space/gpi/gaspi.hpp>
#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/global/topology.hpp>

#include <gpi-space/pc/url.hpp>

#include <we/type/range.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

#include <functional>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      gpi_area_t::gpi_area_t ( fhg::log::Logger& logger
                             , const gpi::pc::type::process_id_t creator
                             , const std::string & name
                             , const gpi::pc::type::flags_t flags
                             , gpi::pc::global::itopology_t & topology
                             , handle_generator_t& handle_generator
                             , api::gaspi_t& gaspi
                             )
        : area_t ( logger
                 , gpi_area_t::area_type
                 , creator
                 , name
                 , gaspi.memory_size()
                 , flags
                 , handle_generator
                 )
        , m_ptr (gaspi.dma_ptr())
        , m_num_com_buffers (8)
        , m_com_buffer_size (4* (1<<20))
        , _topology (topology)
        , _gaspi (gaspi)
      {}

      void gpi_area_t::init ()
      {
        fhg_assert (m_num_com_buffers > 0);
        fhg_assert (m_com_buffer_size > 0);

        // TODO: make  this lazy, just define  a maximum number  of buffers, but
        // try to allocate them only when actually needed.
        size_t num_buffers_allocated = 0;
        for (size_t i = 0; i < m_num_com_buffers; ++i)
        {
          const std::string hdl_name =
            name () + "-com-" + boost::lexical_cast<std::string>(i);
          try
          {
            gpi::pc::type::handle_t com_hdl =
              this->alloc ( GPI_PC_INVAL
                          , m_com_buffer_size
                          , hdl_name
                          , gpi::pc::F_EXCLUSIVE
                          );
            gpi::pc::type::handle::descriptor_t desc =
              descriptor (com_hdl);

            m_com_handles.put (handle_buffer_t ( com_hdl
                                               , desc.local_size
                                               , (char*)m_ptr + desc.offset
                                               )
                              );

            ++num_buffers_allocated;
          }
          catch (std::exception const & ex)
          {
            LLOG (WARN, _logger
                 , "could not allocate communication buffer "
                 << (num_buffers_allocated+1)
                 << ": " << ex.what ()
                 );
            break;
          }
        }

        if (0 == num_buffers_allocated)
        {
          throw std::runtime_error
            ( std::string ("no communication buffer could be allocated:")
            + " com-size := " + boost::lexical_cast<std::string>(m_com_buffer_size)
            + " mem-size := " + boost::lexical_cast<std::string>(descriptor ().local_size)
            );
        }
        else if (descriptor().avail == 0)
        {
          LLOG ( WARN
               , _logger
              ,  "communication buffers consumed all your precious memory,"
              << " this might not be what you wanted!"
              );
        }
      }

      Arena_t
      gpi_area_t::grow_direction (const gpi::pc::type::flags_t flgs) const
      {
        return gpi::flag::is_set (flgs, gpi::pc::F_GLOBAL)
          ? ARENA_UP : ARENA_DOWN;
      }

      void *
      gpi_area_t::raw_ptr (gpi::pc::type::offset_t off)
      {
        return
          (m_ptr && off < descriptor ().local_size)
          ? ((char*)m_ptr + off)
          : (char*)nullptr;
      }

      bool
      gpi_area_t::is_allowed_to_attach (const gpi::pc::type::process_id_t) const
      {
        return false;
      }

      void
      gpi_area_t::alloc_hook (const gpi::pc::type::handle::descriptor_t &hdl)
      {
        if (  gpi::flag::is_set (hdl.flags, gpi::pc::F_GLOBAL)
           && hdl.creator != (gpi::pc::type::process_id_t)(-1)
           )
        {
          _topology.alloc ( descriptor ().id
                                            , hdl.id
                                            , hdl.offset
                                            , hdl.size
                                            , hdl.local_size
                                            , hdl.name
                                            );
        }
      }

      void
      gpi_area_t::free_hook (const gpi::pc::type::handle::descriptor_t &hdl)
      {
        if (gpi::flag::is_set (hdl.flags, gpi::pc::F_GLOBAL))
        {
          _topology.free(hdl.id);
        }
      }

      bool
      gpi_area_t::is_range_local( const gpi::pc::type::handle::descriptor_t &hdl
                                , const gpi::pc::type::offset_t begin
                                , const gpi::pc::type::offset_t end
                                ) const
      {
        gpi::pc::type::id_t     my_rank = _gaspi.rank ();

        if (not gpi::flag::is_set (hdl.flags, gpi::pc::F_GLOBAL))
          my_rank = 0;

        // my part of the handle is within [my_begin, my_end)
        gpi::pc::type::offset_t my_begin =  my_rank      * hdl.local_size;
        gpi::pc::type::offset_t my_end   = (my_rank + 1) * hdl.local_size;

        if (my_end > hdl.size)
          my_end = hdl.size;

        const bool is_local = my_begin <= begin && end <= my_end;

        return is_local;
      }

      gpi::pc::type::size_t
      gpi_area_t::get_local_size ( const gpi::pc::type::size_t size
                                 , const gpi::pc::type::flags_t flgs
                                 ) const
      {
        if (gpi::flag::is_set (flgs, gpi::pc::F_GLOBAL))
        {
          // static distribution scheme with overhead
          const size_t num_nodes = _gaspi.number_of_nodes ();
          size_t overhead = (0 != (size % num_nodes)) ? 1 : 0;
          return (size / num_nodes + overhead);
        }
        else
        {
          return size;
        }
      }

      namespace helper
      {
        namespace
        {
          struct part
          {
            type::offset_t local_offset;
            type::offset_t remote_offset;
            type::size_t size;
            type::rank_t rank;
          };

          //! \todo lazy iteration, …
          std::list<part> split_by_rank
            ( gpi::pc::type::size_t local_offset
            , const gpi::pc::type::size_t remote_base
            , gpi::pc::type::size_t offset
            , const gpi::pc::type::size_t per_node_size
            , gpi::pc::type::size_t amount
            )
          {
            std::list<part> parts;

            while (amount)
            {
              part p;

              p.local_offset = local_offset;
              p.rank = offset / per_node_size;

              const std::size_t max_offset_on_rank
                ((p.rank + 1) * per_node_size);

              p.size = std::min ( std::min (per_node_size, amount)
                                , max_offset_on_rank - offset
                                );
              p.remote_offset = remote_base + (offset % per_node_size);

              parts.emplace_back (p);

              local_offset += p.size;
              offset += p.size;
              amount -= p.size;
            }

            return parts;
          }

          void dma_read_and_wait_for_readable
            ( const gpi::pc::type::handle::descriptor_t & src_hdl
            , const gpi::pc::type::size_t src_offset
            , const gpi::pc::type::handle::descriptor_t & dst_hdl
            , const gpi::pc::type::size_t dst_offset
            , const gpi::pc::type::size_t amount
            , api::gaspi_t& gaspi
            )
          {
            std::list<api::gaspi_t::read_dma_info> handles;

            for ( auto const& part
                : split_by_rank ( dst_hdl.offset + dst_offset
                                , src_hdl.offset
                                , src_offset
                                , src_hdl.local_size
                                , amount
                                )
                )
            {
              handles.emplace_back
                ( gaspi.read_dma ( part.local_offset
                                 , part.remote_offset
                                 , part.size
                                 , part.rank
                                 )
                );
            }

            gaspi.wait_readable (handles);
          }

          std::list<api::gaspi_t::write_dma_info> do_write_dma
            ( const gpi::pc::type::handle::descriptor_t & src_hdl
            , const gpi::pc::type::size_t src_offset
            , const gpi::pc::type::handle::descriptor_t & dst_hdl
            , const gpi::pc::type::size_t dst_offset
            , const gpi::pc::type::size_t amount
            , api::gaspi_t& gaspi
            )
          {
            std::list<api::gaspi_t::write_dma_info> handles;

            for ( auto const& part
                : split_by_rank ( src_hdl.offset + src_offset
                                , dst_hdl.offset
                                , dst_offset
                                , dst_hdl.local_size
                                , amount
                                )
                )
            {
              handles.emplace_back
                ( gaspi.write_dma ( part.local_offset
                                  , part.remote_offset
                                  , part.size
                                  , part.rank
                                  )
                );
            }

            return handles;
          }
        }

        static
        void do_send ( fhg::log::Logger& logger
                     , area_t & src_area
                     , gpi::pc::type::memory_location_t src_loc
                     , gpi_area_t & dst_area
                     , gpi::pc::type::memory_location_t dst_loc
                     , gpi::pc::type::size_t amount
                     , gpi_area_t::handle_pool_t & handle_pool
                     , api::gaspi_t& gaspi
                     )
        {
          handle_buffer_t buf (handle_pool.get());

          gpi::pc::type::size_t remaining = amount;
          while (remaining)
          {
            buf.used (0);

            const gpi::pc::type::size_t to_send =
              std::min (remaining, buf.size ());

            const gpi::pc::type::size_t read_bytes =
              src_area.read_from (src_loc, buf.data (), to_send);
            buf.used (read_bytes);

            if (0 == read_bytes)
            {
              LLOG ( WARN
                   , logger
                   , "could not read from src area - premature end-of-file?"
                   );
              break;
            }

            gaspi.wait_remote_written
              ( do_write_dma ( dst_area.descriptor (buf.handle ())
                             , 0
                             , dst_area.descriptor (dst_loc.handle)
                             , dst_loc.offset
                             , buf.used ()
                             , gaspi
                             )
              );

            src_loc.offset += buf.used ();
            dst_loc.offset += buf.used ();
            remaining      -= buf.used ();
          }

          handle_pool.put (buf);
        }

        static
        void do_recv ( fhg::log::Logger& logger
                     , area_t & dst_area
                     , gpi::pc::type::memory_location_t dst_loc
                     , gpi_area_t & src_area
                     , gpi::pc::type::memory_location_t src_loc
                     , gpi::pc::type::size_t amount
                     , gpi_area_t::handle_pool_t & handle_pool
                     , api::gaspi_t& gaspi
                     )
        {
          handle_buffer_t buf (handle_pool.get());

          gpi::pc::type::size_t remaining = amount;
          while (remaining)
          {
            buf.used (0);

            const gpi::pc::type::size_t to_recv =
              std::min (remaining, buf.size ());

            dma_read_and_wait_for_readable ( src_area.descriptor (src_loc.handle)
                        , src_loc.offset
                        , src_area.descriptor (buf.handle ())
                        , 0
                        , to_recv
                        , gaspi
                        );
            buf.used (to_recv);

            const gpi::pc::type::size_t written_bytes =
              dst_area.write_to (dst_loc, buf.data (), buf.used ());

            if (written_bytes != buf.used ())
            {
              LLOG ( WARN
                   , logger
                   , "could not write to dst area - premature end-of-file?"
                   );
              break;
            }

            src_loc.offset += buf.used ();
            dst_loc.offset += buf.used ();
            remaining      -= buf.used ();
          }

          handle_pool.put (buf);
        }
      }

      std::packaged_task<void()> gpi_area_t::get_specific_transfer_task
        ( const gpi::pc::type::memory_location_t src
        , const gpi::pc::type::memory_location_t dst
        , area_t & dst_area
        , gpi::pc::type::size_t amount
        )
      {
        fhg_assert (type () == dst_area.type ());

        if (is_local (gpi::pc::type::memory_region_t (src, amount)))
        {
          // write dma
          return std::packaged_task<void()>
            ( [this, &dst_area, src, dst, amount]
              {
                _gaspi.wait_remote_written
                  ( helper::do_write_dma ( descriptor (src.handle)
                                         , src.offset
                                         , dst_area.descriptor (dst.handle)
                                         , dst.offset
                                         , amount
                                         , _gaspi
                                         )
                  );
              }
            );
        }
        else if (dst_area.is_local (gpi::pc::type::memory_region_t (dst, amount)))
        {
          // read dma
          return std::packaged_task<void()>
            ( std::bind ( &helper::dma_read_and_wait_for_readable
                        , this->descriptor (src.handle)
                        , src.offset
                        , dst_area.descriptor (dst.handle)
                        , dst.offset
                        , amount
                        , std::ref (_gaspi)
                        )
            );
        }

        throw std::runtime_error
          ( "illegal memory transfer requested:"
          " source and destination cannot both be remote!"
          );
      }

      std::packaged_task<void()> gpi_area_t::get_send_task
        ( area_t & src_area
        , const gpi::pc::type::memory_location_t src
        , const gpi::pc::type::memory_location_t dst
        , gpi::pc::type::size_t amount
        )
      {
        return std::packaged_task<void()>
          ( [this, &src_area, src, dst, amount]
            {
              helper::do_send ( _logger
                              , src_area
                              , src
                              , *this
                              , dst
                              , amount
                              , m_com_handles
                              , _gaspi
                              );
            }
          );
      }

      std::packaged_task<void()> gpi_area_t::get_recv_task
        ( area_t & dst_area
        , const gpi::pc::type::memory_location_t dst
        , const gpi::pc::type::memory_location_t src
        , gpi::pc::type::size_t amount
        )
      {
        return std::packaged_task<void()>
          ( std::bind ( &helper::do_recv
                      , std::ref (_logger)
                      , std::ref (dst_area)
                      , dst
                      , std::ref (*this)
                      , src
                      , amount
                      , std::ref (m_com_handles)
                      , std::ref (_gaspi)
                      )
          );
      }

      double gpi_area_t::get_transfer_costs ( const gpi::pc::type::memory_region_t& transfer
                                            , const gpi::rank_t rank
                                            ) const
      {
        const gpi::pc::type::handle::descriptor_t allocation
          (descriptor (transfer.location.handle));

        we::interval const local_part
          ( we::interval ( transfer.location.offset
                         , transfer.size
                         ).intersect
          ( we::interval ( rank * allocation.local_size
                         , allocation.local_size
                         )
          ));

        constexpr const double remote_transfer_cost_weight {1.0};

        return transfer.size + remote_transfer_cost_weight * (transfer.size - local_part.size());
      }

      area_ptr_t gpi_area_t::create
        ( fhg::log::Logger& logger
        , std::string const &url_s
        , gpi::pc::global::itopology_t & topology
        , handle_generator_t& handle_generator
        , api::gaspi_t& gaspi
        )
      {
        url_t url (url_s);

        type::size_t comsize =
          boost::lexical_cast<type::size_t>(url.get ("buffer_size").get_value_or ("4194304"));
        type::size_t numbuf =
          boost::lexical_cast<type::size_t>(url.get ("buffers").get_value_or ("8"));

        gpi_area_t * area = new gpi_area_t ( logger
                                           , GPI_PC_INVAL
                                           , "GPI"
                                           , gpi::pc::F_PERSISTENT
                                           + gpi::pc::F_GLOBAL
                                           , topology
                                           , handle_generator
                                           , gaspi
                                           );
        area->m_num_com_buffers = numbuf;
        area->m_com_buffer_size = comsize;
        return area_ptr_t (area);
      }
    }
  }
}
