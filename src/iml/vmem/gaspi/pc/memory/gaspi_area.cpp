// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <iml/vmem/gaspi/pc/memory/gaspi_area.hpp>

#include <utility>

#include <iml/MemoryOffset.hpp>
#include <iml/MemorySize.hpp>
#include <iml/vmem/gaspi/gpi/gaspi.hpp>
#include <iml/vmem/gaspi/pc/global/topology.hpp>
#include <iml/vmem/gaspi/pc/type/impl_types.hpp>

#include <util-generic/divru.hpp>
#include <util-generic/print_exception.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

#include <functional>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      gaspi_area_t::gaspi_area_t ( gpi::pc::global::itopology_t & topology
                                 , handle_generator_t& handle_generator
                                 , fhg::iml::vmem::gaspi_context& gaspi_context
                                 , fhg::iml::vmem::gaspi_timeout& time_left
                                 , iml::MemorySize per_node_size
                                 , iml::MemorySize num_com_buffers
                                 , iml::MemorySize com_buffer_size
                                 , iml::SegmentHandle segment_id
                                 )
        : area_t (per_node_size)
        , _gaspi_context (gaspi_context)
        , _gaspi (_gaspi_context, per_node_size, time_left)
        , m_ptr (_gaspi.dma_ptr())
        , m_num_com_buffers (num_com_buffers)
        , m_com_buffer_size (com_buffer_size)
        , _topology (topology)
      {
        if (m_num_com_buffers == 0)
        {
          throw std::invalid_argument
            ("at least one communciation buffer is required");
        }
        if (m_com_buffer_size == 0)
        {
          throw std::invalid_argument
            ("communication buffers can't be zero sized");
        }

        //! \todo move to different gaspi segment to have separate
        //! allocation and not operate in the user's one. this could
        //! then also be used to reduce the buffer count when having
        //! multiple areas.
        std::size_t num_buffers_allocated = 0;
        for (std::size_t i = 0; i < m_num_com_buffers; ++i)
        {
          try
          {
            auto const com_hdl (handle_generator.next_allocation());
              this->alloc ( m_com_buffer_size
                          , is_global::no
                          , segment_id
                          , com_hdl
                          );
            gpi::pc::type::handle::descriptor_t desc =
              descriptor (com_hdl);

            m_com_handles.put (handle_buffer_t ( com_hdl
                                               , desc.local_size
                                               , static_cast<char*> (m_ptr) + desc.offset
                                               )
                              );

            ++num_buffers_allocated;
          }
          catch (...)
          {
            break;
          }
        }

        if (m_num_com_buffers != num_buffers_allocated)
        {
          throw std::runtime_error
            ( std::string ("not all communication buffers could be allocated:")
            + " com-size := " + boost::lexical_cast<std::string>(m_com_buffer_size)
            + " mem-size := " + boost::lexical_cast<std::string>(_local_size)
            );
        }
      }

      void *
      gaspi_area_t::raw_ptr (iml::MemoryOffset off)
      {
        return
          (m_ptr && off < _local_size)
          ? (static_cast<char*> (m_ptr) + off)
          : nullptr;
      }

      global::itopology_t& gaspi_area_t::global_topology()
      {
        return _topology;
      }

      bool
      gaspi_area_t::is_range_local( gpi::pc::type::handle::descriptor_t const& hdl
                                  , iml::MemoryOffset begin
                                  , iml::MemoryOffset end
                                  ) const
      {
        auto my_rank = _gaspi_context.rank ();

        if (hdl.flags == is_global::no)
          my_rank = 0;

        // my part of the handle is within [my_begin, my_end)
        iml::MemoryOffset my_begin =  my_rank        * hdl.local_size;
        iml::MemoryOffset my_end   = (my_rank + 1ul) * hdl.local_size;

        if (my_end > hdl.size)
          my_end = hdl.size;

        const bool is_local = my_begin <= begin && end <= my_end;

        return is_local;
      }

      iml::MemorySize
      gaspi_area_t::get_local_size ( iml::MemorySize size
                                   , gpi::pc::type::flags_t flgs
                                   ) const
      {
        if (flgs == is_global::yes)
        {
          // static distribution scheme with overhead
          const std::size_t num_nodes = _gaspi_context.number_of_nodes ();
          std::size_t overhead = (0 != (size % num_nodes)) ? 1 : 0;
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
            ( iml::MemorySize local_offset
            , iml::MemorySize remote_base
            , iml::MemorySize offset
            , iml::MemorySize per_node_size
            , iml::MemorySize amount
            )
          {
            api::gaspi_t::transfers_t parts;

            while (amount)
            {
              api::gaspi_t::transfer_part p;

              p.local_offset = local_offset;
              p.rank = offset / per_node_size;

              const std::size_t max_offset_on_rank
                ((p.rank + 1ul) * per_node_size);

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
            ( gpi::pc::type::handle::descriptor_t const& src_hdl
            , iml::MemorySize src_offset
            , gpi::pc::type::handle::descriptor_t const& dst_hdl
            , iml::MemorySize dst_offset
            , iml::MemorySize amount
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
            ( gpi::pc::type::handle::descriptor_t const& src_hdl
            , iml::MemorySize src_offset
            , gpi::pc::type::handle::descriptor_t const& dst_hdl
            , iml::MemorySize dst_offset
            , iml::MemorySize amount
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
        void do_send ( area_t & src_area
                     , iml::MemoryLocation src_loc
                     , gaspi_area_t & dst_area
                     , iml::MemoryLocation dst_loc
                     , iml::MemorySize amount
                     , gaspi_area_t::handle_pool_t & handle_pool
                     , api::gaspi_t& gaspi
                     )
        {
          handle_buffer_t buf (handle_pool.get());

          iml::MemorySize remaining = amount;
          while (remaining)
          {
            buf.used (0);

            const iml::MemorySize to_send =
              std::min (remaining, buf.size ());

            const iml::MemorySize read_bytes =
              src_area.read_from (src_loc, buf.data (), to_send);
            buf.used (read_bytes);

            if (0 == read_bytes)
            {
              throw std::runtime_error
                ( "could not read from src area - premature "
                  "end-of-file?"
                );
            }

            do_write_dma_and_wait_remote_written
              ( dst_area.descriptor (buf.handle ())
              , 0
              , dst_area.descriptor (dst_loc.allocation)
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
        void do_recv ( area_t & dst_area
                     , iml::MemoryLocation dst_loc
                     , gaspi_area_t & src_area
                     , iml::MemoryLocation src_loc
                     , iml::MemorySize amount
                     , gaspi_area_t::handle_pool_t & handle_pool
                     , api::gaspi_t& gaspi
                     )
        {
          handle_buffer_t buf (handle_pool.get());

          iml::MemorySize remaining = amount;
          while (remaining)
          {
            buf.used (0);

            const iml::MemorySize to_recv =
              std::min (remaining, buf.size ());

            dma_read_and_wait_for_readable ( src_area.descriptor (src_loc.allocation)
                        , src_loc.offset
                        , src_area.descriptor (buf.handle ())
                        , 0
                        , to_recv
                        , gaspi
                        );
            buf.used (to_recv);

            const iml::MemorySize written_bytes =
              dst_area.write_to (dst_loc, buf.data (), buf.used ());

            if (written_bytes != buf.used ())
            {
              throw std::runtime_error
                ( "could not write to dst area - premature "
                  "end-of-file?"
                );
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
        , iml::MemoryLocation src
        , iml::MemoryLocation dst
        , iml::MemorySize amount
        )
      {
        return std::packaged_task<void()>
          ( [this, &src_area, src, dst, amount]
            {
              helper::do_send ( src_area
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
        , iml::MemoryLocation dst
        , iml::MemoryLocation src
        , iml::MemorySize amount
        )
      {
        return std::packaged_task<void()>
          ( std::bind ( &helper::do_recv
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

      namespace
      {
        iml::MemorySize size_of_intersection
          ( iml::MemoryOffset a_begin, iml::MemorySize a_size
          , iml::MemoryOffset b_begin, iml::MemorySize b_size
          )
        {
          auto const a_end (a_begin + a_size);
          auto const b_end (b_begin + b_size);

          auto const begin (std::max (a_begin, b_begin));
          auto const end (std::min (a_end, b_end));

          return end > begin ? end - begin : 0;
        }
      }

      double gaspi_area_t::get_transfer_costs ( iml::MemoryRegion const& transfer
                                              , gpi::rank_t rank
                                              ) const
      {
        const gpi::pc::type::handle::descriptor_t allocation
          (descriptor (transfer.allocation));

        auto const local_size
          ( size_of_intersection
              ( transfer.offset, transfer.size
              , rank * allocation.local_size, allocation.local_size
              )
          );

        constexpr const double remote_transfer_cost_weight {1.0};

        return transfer.size + remote_transfer_cost_weight * (transfer.size - local_size);
      }

      area_ptr_t gaspi_area_t::create
        ( iml::gaspi::SegmentDescription const& description
        , unsigned long total_size
        , gpi::pc::global::itopology_t & topology
        , handle_generator_t& handle_generator
        , fhg::iml::vmem::gaspi_context& gaspi_context
        , iml::SegmentHandle segment_id
        )
      {
        iml::MemorySize comsize = description.communication_buffer_size;
        iml::MemorySize numbuf = description.communication_buffer_count;
        iml::MemorySize const per_node_size
          ( fhg::util::divru (total_size, gaspi_context.number_of_nodes())
          + comsize * numbuf
          );

        //! \todo get from user? use for other areas as well? remove?
        fhg::iml::vmem::gaspi_timeout time_left (std::chrono::seconds (30));
        gaspi_area_t * area = new gaspi_area_t ( topology
                                               , handle_generator
                                               , gaspi_context
                                               , time_left
                                               , per_node_size
                                               , numbuf
                                               , comsize
                                               , segment_id
                                               );
        return area_ptr_t (area);
      }
    }
  }
}
