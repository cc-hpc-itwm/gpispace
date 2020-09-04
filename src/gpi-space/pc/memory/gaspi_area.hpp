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

#pragma once

#include <gpi-space/gpi/gaspi.hpp>

#include <gpi-space/pc/type/segment_type.hpp>
#include <gpi-space/pc/memory/memory_area.hpp>
#include <gpi-space/pc/memory/handle_buffer.hpp>

#include <gpi-space/pc/global/itopology.hpp>

#include <vmem/gaspi_context.hpp>

#include <fhg/util/thread/queue.hpp>

namespace gpi
{
  namespace api
  {
    class gaspi_t;
  }
  namespace pc
  {
    namespace memory
    {
      class gaspi_area_t : public area_t
      {
      public:
        static const type::segment::segment_type area_type = gpi::pc::type::segment::SEG_GASPI;

        typedef fhg::thread::queue<handle_buffer_t> handle_pool_t;

        static area_ptr_t create ( fhg::logging::stream_emitter&
                                 , std::string const &url
                                 , gpi::pc::global::itopology_t & topology
                                 , handle_generator_t&
                                 , fhg::vmem::gaspi_context&
                                 , type::id_t owner
                                 );

      protected:
        gaspi_area_t ( fhg::logging::stream_emitter&
                     , const gpi::pc::type::process_id_t creator
                     , const std::string & name
                     , const gpi::pc::type::flags_t flags
                     , gpi::pc::global::itopology_t & topology
                     , handle_generator_t&
                     , fhg::vmem::gaspi_context&
                     , fhg::vmem::gaspi_timeout&
                     , type::size_t memory_size
                     , type::size_t num_com_buffers
                     , type::size_t com_buffer_size
                     );

        virtual bool is_allowed_to_attach (const gpi::pc::type::process_id_t) const override;
        virtual gspc::vmem::dtmmgr::Arena_t grow_direction (const gpi::pc::type::flags_t) const override;

        virtual void alloc_hook (const gpi::pc::type::handle::descriptor_t &) override;
        virtual void  free_hook (const gpi::pc::type::handle::descriptor_t &) override;

        virtual std::packaged_task<void()> get_send_task
          ( area_t & src_area
          , const gpi::pc::type::memory_location_t src
          , const gpi::pc::type::memory_location_t dst
          , gpi::pc::type::size_t amount
          ) override;

        virtual std::packaged_task<void()> get_recv_task
          ( area_t & dst_area
          , const gpi::pc::type::memory_location_t dst
          , const gpi::pc::type::memory_location_t src
          , gpi::pc::type::size_t amount
          ) override;

      private:
        virtual bool is_range_local ( const gpi::pc::type::handle::descriptor_t &
                            , const gpi::pc::type::offset_t begin
                            , const gpi::pc::type::size_t   range_size
                            ) const override;

        gpi::pc::type::offset_t
        handle_to_global_offset ( const gpi::pc::type::offset_t
                                , const gpi::pc::type::size_t per_node_size
                                ) const;

        virtual void *raw_ptr (gpi::pc::type::offset_t off) override;

        virtual gpi::pc::type::size_t get_local_size ( const gpi::pc::type::size_t size
                                             , const gpi::pc::type::flags_t flags
                                             ) const override;

        double get_transfer_costs ( const gpi::pc::type::memory_region_t&
                                  , const gpi::rank_t
                                  ) const override;

        fhg::vmem::gaspi_context& _gaspi_context;
        api::gaspi_t _gaspi;

        void * m_ptr;

        handle_pool_t m_com_handles; // local allocations that can be used to transfer data

        gpi::pc::type::size_t m_num_com_buffers;
        gpi::pc::type::size_t m_com_buffer_size;

        gpi::pc::global::itopology_t & _topology;
      };
    }
  }
}
