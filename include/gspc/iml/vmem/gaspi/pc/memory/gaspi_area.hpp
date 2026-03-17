// Copyright (C) 2011-2012,2014-2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/MemoryOffset.hpp>
#include <gspc/iml/MemorySize.hpp>
#include <gspc/iml/gaspi/SegmentDescription.hpp>
#include <gspc/iml/vmem/gaspi/gpi/gaspi.hpp>
#include <gspc/iml/vmem/gaspi/pc/global/itopology.hpp>
#include <gspc/iml/vmem/gaspi/pc/memory/handle_buffer.hpp>
#include <gspc/iml/vmem/gaspi/pc/memory/handle_generator.hpp>
#include <gspc/iml/vmem/gaspi/pc/memory/memory_area.hpp>
#include <gspc/iml/vmem/gaspi_context.hpp>

#include <gspc/util/threadsafe_queue.hpp>

namespace gpi
{
  namespace api
  {
    class gaspi_t;
  }

    namespace pc::memory
    {
      class gaspi_area_t : public area_t
      {
      public:
        using handle_pool_t = gspc::util::threadsafe_queue<handle_buffer_t>;

        static area_ptr_t create ( gspc::iml::gaspi::SegmentDescription const&
                                 , unsigned long total_size
                                 , gpi::pc::global::itopology_t & topology
                                 , handle_generator_t&
                                 , gspc::iml::vmem::gaspi_context&
                                 , gspc::iml::SegmentHandle segment_id
                                 );

      protected:
        gaspi_area_t ( gpi::pc::global::itopology_t & topology
                     , handle_generator_t&
                     , gspc::iml::vmem::gaspi_context&
                     , gspc::iml::vmem::gaspi_timeout&
                     , gspc::iml::MemorySize memory_size
                     , gspc::iml::MemorySize num_com_buffers
                     , gspc::iml::MemorySize com_buffer_size
                     , gspc::iml::SegmentHandle segment_id
                     );

        global::itopology_t& global_topology() override;

        std::packaged_task<void()> get_send_task
          ( area_t & src_area
          , gspc::iml::MemoryLocation src
          , gspc::iml::MemoryLocation dst
          , gspc::iml::MemorySize amount
          ) override;

        std::packaged_task<void()> get_recv_task
          ( area_t & dst_area
          , gspc::iml::MemoryLocation dst
          , gspc::iml::MemoryLocation src
          , gspc::iml::MemorySize amount
          ) override;

      private:
        bool is_range_local ( gpi::pc::type::handle::descriptor_t const&
                            , gspc::iml::MemoryOffset begin
                            , gspc::iml::MemorySize   range_size
                            ) const override;

        gspc::iml::MemoryOffset
        handle_to_global_offset ( gspc::iml::MemoryOffset
                                , gspc::iml::MemorySize per_node_size
                                ) const;

        void *raw_ptr (gspc::iml::MemoryOffset off) override;

        gspc::iml::MemorySize get_local_size ( gspc::iml::MemorySize size
                                             , gpi::pc::type::flags_t flags
                                             ) const override;

        double get_transfer_costs ( gspc::iml::MemoryRegion const&
                                  , gpi::rank_t
                                  ) const override;

        gspc::iml::vmem::gaspi_context& _gaspi_context;
        api::gaspi_t _gaspi;

        void * m_ptr;

        handle_pool_t m_com_handles; // local allocations that can be used to transfer data

        gspc::iml::MemorySize m_num_com_buffers;
        gspc::iml::MemorySize m_com_buffer_size;

        gpi::pc::global::itopology_t & _topology;
      };
    }

}
