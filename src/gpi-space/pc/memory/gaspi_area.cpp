// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <gpi-space/pc/memory/gaspi_area.hpp>

#include <utility>

#include <fhg/assert.hpp>
#include <gpi-space/gpi/gaspi.hpp>
#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/global/topology.hpp>

#include <gpi-space/pc/url.hpp>

#include <util-generic/divru.hpp>
#include <util-generic/print_exception.hpp>

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
      gaspi_area_t::gaspi_area_t ( fhg::logging::stream_emitter& logger
                                 , const gpi::pc::type::process_id_t creator
                                 , const std::string & name
                                 , const gpi::pc::type::flags_t flags
                                 , gpi::pc::global::itopology_t & topology
                                 , handle_generator_t& handle_generator
                                 , fhg::vmem::gaspi_context& gaspi_context
                                 , fhg::vmem::gaspi_timeout& time_left
                                 , type::size_t per_node_size
                                 , type::size_t num_com_buffers
                                 , type::size_t com_buffer_size
                                 )
        : area_t ( logger
                 , gaspi_area_t::area_type
                 , creator
                 , name
                 , per_node_size
                 , flags
                 , handle_generator
                 )
        , _gaspi_context (gaspi_context)
        , _gaspi (_gaspi_context, logger, per_node_size, time_left)
        , m_ptr (_gaspi.dma_ptr())
        , m_num_com_buffers (num_com_buffers)
        , m_com_buffer_size (com_buffer_size)
        , _topology (topology)
      {
        fhg_assert (m_num_com_buffers > 0);
        fhg_assert (m_com_buffer_size > 0);

        //! \todo move to different gaspi segment to have separate
        //! allocation and not operate in the user's one. this could
        //! then also be used to reduce the buffer count when having
        //! multiple areas.
        size_t num_buffers_allocated = 0;
        for (size_t i = 0; i < m_num_com_buffers; ++i)
        {
          const std::string hdl_name =
            name + "-com-" + boost::lexical_cast<std::string>(i);
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
          catch (...)
          {
            _logger.emit ( "could not allocate communication buffer "
                         + std::to_string (num_buffers_allocated + 1) + ": "
                         + fhg::util::current_exception_printer().string()
                         , fhg::logging::legacy::category_level_warn
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
          _logger.emit ( "communication buffers consumed all your precious "
                         "memory, this might not be what you wanted!"
                       , fhg::logging::legacy::category_level_warn
                       );
        }
      }

      gspc::vmem::dtmmgr::Arena_t
      gaspi_area_t::grow_direction (const gpi::pc::type::flags_t flgs) const
      {
        return gpi::flag::is_set (flgs, gpi::pc::F_GLOBAL)
          ? gspc::vmem::dtmmgr::ARENA_UP : gspc::vmem::dtmmgr::ARENA_DOWN;
      }

      void *
      gaspi_area_t::raw_ptr (gpi::pc::type::offset_t off)
      {
        return
          (m_ptr && off < descriptor ().local_size)
          ? ((char*)m_ptr + off)
          : (char*)nullptr;
      }

      bool
      gaspi_area_t::is_allowed_to_attach (const gpi::pc::type::process_id_t) const
      {
        return false;
      }

      void
      gaspi_area_t::alloc_hook (const gpi::pc::type::handle::descriptor_t &hdl)
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
      gaspi_area_t::free_hook (const gpi::pc::type::handle::descriptor_t &hdl)
      {
        if (gpi::flag::is_set (hdl.flags, gpi::pc::F_GLOBAL))
        {
          _topology.free(hdl.id);
        }
      }

      bool
      gaspi_area_t::is_range_local( const gpi::pc::type::handle::descriptor_t &hdl
                                  , const gpi::pc::type::offset_t begin
                                  , const gpi::pc::type::offset_t end
                                  ) const
      {
        gpi::pc::type::id_t     my_rank = _gaspi_context.rank ();

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
      gaspi_area_t::get_local_size ( const gpi::pc::type::size_t size
                                   , const gpi::pc::type::flags_t flgs
                                   ) const
      {
        if (gpi::flag::is_set (flgs, gpi::pc::F_GLOBAL))
        {
          // static distribution scheme with overhead
          const size_t num_nodes = _gaspi_context.number_of_nodes ();
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
          //! \todo lazy iteration, …
          api::gaspi_t::transfers_t split_by_rank
            ( gpi::pc::type::size_t local_offset
            , const gpi::pc::type::size_t remote_base
            , gpi::pc::type::size_t offset
            , const gpi::pc::type::size_t per_node_size
            , gpi::pc::type::size_t amount
            )
          {
            api::gaspi_t::transfers_t parts;

            while (amount)
            {
              api::gaspi_t::transfer_part p;

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
            gaspi.wait_readable
              ( gaspi.read_dma ( split_by_rank ( dst_hdl.offset + dst_offset
                                               , src_hdl.offset
                                               , src_offset
                                               , src_hdl.local_size
                                               , amount
                                               )
                               )
              );
          }

          void do_write_dma_and_wait_remote_written
            ( const gpi::pc::type::handle::descriptor_t & src_hdl
            , const gpi::pc::type::size_t src_offset
            , const gpi::pc::type::handle::descriptor_t & dst_hdl
            , const gpi::pc::type::size_t dst_offset
            , const gpi::pc::type::size_t amount
            , api::gaspi_t& gaspi
            )
          {
            gaspi.wait_remote_written
              ( gaspi.write_dma ( split_by_rank ( src_hdl.offset + src_offset
                                                , dst_hdl.offset
                                                , dst_offset
                                                , dst_hdl.local_size
                                                , amount
                                                )
                                )
              );
          }
        }

        static
        void do_send ( fhg::logging::stream_emitter& logger
                     , area_t & src_area
                     , gpi::pc::type::memory_location_t src_loc
                     , gaspi_area_t & dst_area
                     , gpi::pc::type::memory_location_t dst_loc
                     , gpi::pc::type::size_t amount
                     , gaspi_area_t::handle_pool_t & handle_pool
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
              logger.emit ( "could not read from src area - premature "
                            "end-of-file?"
                          , fhg::logging::legacy::category_level_warn
                          );
              break;
            }

            do_write_dma_and_wait_remote_written
              ( dst_area.descriptor (buf.handle ())
              , 0
              , dst_area.descriptor (dst_loc.handle)
              , dst_loc.offset
              , buf.used ()
              , gaspi
              );

            src_loc.offset += buf.used ();
            dst_loc.offset += buf.used ();
            remaining      -= buf.used ();
          }

          handle_pool.put (buf);
        }

        static
        void do_recv ( fhg::logging::stream_emitter& logger
                     , area_t & dst_area
                     , gpi::pc::type::memory_location_t dst_loc
                     , gaspi_area_t & src_area
                     , gpi::pc::type::memory_location_t src_loc
                     , gpi::pc::type::size_t amount
                     , gaspi_area_t::handle_pool_t & handle_pool
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
              logger.emit ( "could not write to dst area - premature "
                            "end-of-file?"
                          , fhg::logging::legacy::category_level_warn
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

      std::packaged_task<void()> gaspi_area_t::get_send_task
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

      std::packaged_task<void()> gaspi_area_t::get_recv_task
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

      double gaspi_area_t::get_transfer_costs ( const gpi::pc::type::memory_region_t& transfer
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

      area_ptr_t gaspi_area_t::create
        ( fhg::logging::stream_emitter& logger
        , std::string const &url_s
        , gpi::pc::global::itopology_t & topology
        , handle_generator_t& handle_generator
        , fhg::vmem::gaspi_context& gaspi_context
        , type::id_t owner
        )
      {
        url_t url (url_s);

        type::size_t comsize =
          boost::lexical_cast<type::size_t>(url.get ("buffer_size").get_value_or ("4194304"));
        type::size_t numbuf =
          boost::lexical_cast<type::size_t>(url.get ("buffers").get_value_or ("8"));
        type::size_t const total_size
          (boost::lexical_cast<type::size_t> (url.get ("total_size").get()));
        type::size_t const per_node_size
          ( fhg::util::divru (total_size, gaspi_context.number_of_nodes())
          + comsize * numbuf
          );

        //! \todo get from user? use for other areas as well? remove?
        fhg::vmem::gaspi_timeout time_left (std::chrono::seconds (30));
        gaspi_area_t * area = new gaspi_area_t ( logger
                                               , owner
                                               , "GASPI"
                                               , gpi::pc::F_GLOBAL
                                               , topology
                                               , handle_generator
                                               , gaspi_context
                                               , time_left
                                               , per_node_size
                                               , numbuf
                                               , comsize
                                               );
        return area_ptr_t (area);
      }
    }
  }
}
