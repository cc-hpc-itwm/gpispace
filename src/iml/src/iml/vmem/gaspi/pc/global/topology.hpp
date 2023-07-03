// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/AllocationHandle.hpp>
#include <iml/MemoryOffset.hpp>
#include <iml/MemorySize.hpp>
#include <iml/SegmentDescription.hpp>
#include <iml/SegmentHandle.hpp>
#include <iml/vmem/gaspi/pc/global/itopology.hpp>
#include <iml/vmem/gaspi/pc/memory/manager.hpp>
#include <iml/vmem/gaspi_context.hpp>

#include <util-rpc/function_description.hpp>
#include <util-rpc/remote_tcp_endpoint.hpp>
#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>
#include <util-rpc/service_tcp_provider.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <list>
#include <memory>
#include <mutex>

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      class topology_t : public itopology_t
      {
      public:
        topology_t ( memory::manager_t& memory_manager
                   , fhg::iml::vmem::gaspi_context&
                   , std::unique_ptr<fhg::rpc::service_tcp_provider_with_deferred_dispatcher>
                   );

        ~topology_t() override = default;
        topology_t (topology_t const&) = delete;
        topology_t (topology_t&&) = delete;
        topology_t& operator= (topology_t const&) = delete;
        topology_t& operator= (topology_t&&) = delete;

        void alloc ( iml::SegmentHandle segment
                           , iml::AllocationHandle
                           , iml::MemoryOffset
                           , iml::MemorySize size
                           , iml::MemorySize local_size
                           ) override;

        void free (iml::AllocationHandle) override;

        void add_memory ( iml::SegmentHandle seg_id
                                , iml::SegmentDescription const& description
                                , unsigned long total_size
                                ) override;
        void del_memory (iml::SegmentHandle seg_id) override;

      private:
        std::mutex m_global_alloc_mutex;

        fhg::iml::vmem::gaspi_context& _gaspi_context;

        FHG_RPC_FUNCTION_DESCRIPTION ( alloc_desc
                                     , void ( iml::SegmentHandle
                                            , iml::AllocationHandle
                                            , iml::MemoryOffset
                                            , iml::MemorySize size
                                            , iml::MemorySize local_size
                                            )
                                     );
        FHG_RPC_FUNCTION_DESCRIPTION ( free_desc
                                     , void (iml::AllocationHandle)
                                     );

        FHG_RPC_FUNCTION_DESCRIPTION ( add_memory_desc
                                     , void ( iml::SegmentHandle
                                            , iml::SegmentDescription
                                            , unsigned long
                                            )
                                     );
        FHG_RPC_FUNCTION_DESCRIPTION ( del_memory_desc
                                     , void (iml::SegmentHandle)
                                     );

        fhg::rpc::service_dispatcher _service_dispatcher;
        fhg::rpc::service_handler<alloc_desc> _alloc;
        fhg::rpc::service_handler<free_desc> _free;
        fhg::rpc::service_handler<add_memory_desc> _add_memory;
        fhg::rpc::service_handler<del_memory_desc> _del_memory;

        fhg::util::scoped_boost_asio_io_service_with_threads _client_io_service;
        std::list<fhg::rpc::remote_tcp_endpoint> _others;

        std::unique_ptr<fhg::rpc::service_tcp_provider_with_deferred_dispatcher> _server;

        template<typename Description, typename... Args>
          void request (Args&&...);
      };
    }
  }
}
