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

#pragma once

#include <iml/MemoryOffset.hpp>
#include <iml/MemorySize.hpp>
#include <iml/gaspi/SegmentDescription.hpp>
#include <iml/vmem/gaspi/gpi/gaspi.hpp>
#include <iml/vmem/gaspi/pc/global/itopology.hpp>
#include <iml/vmem/gaspi/pc/memory/handle_buffer.hpp>
#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>
#include <iml/vmem/gaspi/pc/memory/memory_area.hpp>
#include <iml/vmem/gaspi_context.hpp>

#include <util-generic/threadsafe_queue.hpp>

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
        typedef fhg::util::threadsafe_queue<handle_buffer_t> handle_pool_t;

        static area_ptr_t create ( iml::gaspi::SegmentDescription const&
                                 , unsigned long total_size
                                 , gpi::pc::global::itopology_t & topology
                                 , handle_generator_t&
                                 , fhg::iml::vmem::gaspi_context&
                                 , iml::SegmentHandle segment_id
                                 );

      protected:
        gaspi_area_t ( gpi::pc::global::itopology_t & topology
                     , handle_generator_t&
                     , fhg::iml::vmem::gaspi_context&
                     , fhg::iml::vmem::gaspi_timeout&
                     , iml::MemorySize memory_size
                     , iml::MemorySize num_com_buffers
                     , iml::MemorySize com_buffer_size
                     , iml::SegmentHandle segment_id
                     );

        virtual global::itopology_t& global_topology() override;

        virtual std::packaged_task<void()> get_send_task
          ( area_t & src_area
          , iml::MemoryLocation src
          , iml::MemoryLocation dst
          , iml::MemorySize amount
          ) override;

        virtual std::packaged_task<void()> get_recv_task
          ( area_t & dst_area
          , iml::MemoryLocation dst
          , iml::MemoryLocation src
          , iml::MemorySize amount
          ) override;

      private:
        virtual bool is_range_local ( gpi::pc::type::handle::descriptor_t const&
                            , iml::MemoryOffset begin
                            , iml::MemorySize   range_size
                            ) const override;

        iml::MemoryOffset
        handle_to_global_offset ( iml::MemoryOffset
                                , iml::MemorySize per_node_size
                                ) const;

        virtual void *raw_ptr (iml::MemoryOffset off) override;

        virtual iml::MemorySize get_local_size ( iml::MemorySize size
                                             , gpi::pc::type::flags_t flags
                                             ) const override;

        double get_transfer_costs ( iml::MemoryRegion const&
                                  , gpi::rank_t
                                  ) const override;

        fhg::iml::vmem::gaspi_context& _gaspi_context;
        api::gaspi_t _gaspi;

        void * m_ptr;

        handle_pool_t m_com_handles; // local allocations that can be used to transfer data

        iml::MemorySize m_num_com_buffers;
        iml::MemorySize m_com_buffer_size;

        gpi::pc::global::itopology_t & _topology;
      };
    }
  }
}
