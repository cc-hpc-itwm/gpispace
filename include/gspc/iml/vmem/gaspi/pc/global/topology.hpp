// Copyright (C) 2011-2012,2014-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/AllocationHandle.hpp>
#include <gspc/iml/MemoryOffset.hpp>
#include <gspc/iml/MemorySize.hpp>
#include <gspc/iml/SegmentDescription.hpp>
#include <gspc/iml/SegmentHandle.hpp>
#include <gspc/iml/vmem/gaspi/pc/global/itopology.hpp>
#include <gspc/iml/vmem/gaspi/pc/memory/manager.hpp>
#include <gspc/iml/vmem/gaspi_context.hpp>

#include <gspc/rpc/function_description.hpp>
#include <gspc/rpc/remote_tcp_endpoint.hpp>
#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>

#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>

#include <list>
#include <memory>
#include <mutex>



    namespace gpi::pc::global
    {
      class topology_t : public itopology_t
      {
      public:
        topology_t ( memory::manager_t& memory_manager
                   , gspc::iml::vmem::gaspi_context&
                   , std::unique_ptr<gspc::rpc::service_tcp_provider_with_deferred_dispatcher>
                   );

        ~topology_t() override = default;
        topology_t (topology_t const&) = delete;
        topology_t (topology_t&&) = delete;
        topology_t& operator= (topology_t const&) = delete;
        topology_t& operator= (topology_t&&) = delete;

        void alloc ( gspc::iml::SegmentHandle segment
                           , gspc::iml::AllocationHandle
                           , gspc::iml::MemoryOffset
                           , gspc::iml::MemorySize size
                           , gspc::iml::MemorySize local_size
                           ) override;

        void free (gspc::iml::AllocationHandle) override;

        void add_memory ( gspc::iml::SegmentHandle seg_id
                                , gspc::iml::SegmentDescription const& description
                                , unsigned long total_size
                                ) override;
        void del_memory (gspc::iml::SegmentHandle seg_id) override;

      private:
        std::mutex m_global_alloc_mutex;

        gspc::iml::vmem::gaspi_context& _gaspi_context;

        FHG_RPC_FUNCTION_DESCRIPTION ( alloc_desc
                                     , void ( gspc::iml::SegmentHandle
                                            , gspc::iml::AllocationHandle
                                            , gspc::iml::MemoryOffset
                                            , gspc::iml::MemorySize size
                                            , gspc::iml::MemorySize local_size
                                            )
                                     );
        FHG_RPC_FUNCTION_DESCRIPTION ( free_desc
                                     , void (gspc::iml::AllocationHandle)
                                     );

        FHG_RPC_FUNCTION_DESCRIPTION ( add_memory_desc
                                     , void ( gspc::iml::SegmentHandle
                                            , gspc::iml::SegmentDescription
                                            , unsigned long
                                            )
                                     );
        FHG_RPC_FUNCTION_DESCRIPTION ( del_memory_desc
                                     , void (gspc::iml::SegmentHandle)
                                     );

        gspc::rpc::service_dispatcher _service_dispatcher;
        gspc::rpc::service_handler<alloc_desc> _alloc;
        gspc::rpc::service_handler<free_desc> _free;
        gspc::rpc::service_handler<add_memory_desc> _add_memory;
        gspc::rpc::service_handler<del_memory_desc> _del_memory;

        gspc::util::scoped_boost_asio_io_service_with_threads _client_io_service;
        std::list<gspc::rpc::remote_tcp_endpoint> _others;

        std::unique_ptr<gspc::rpc::service_tcp_provider_with_deferred_dispatcher> _server;

        template<typename Description, typename... Args>
          void request (Args&&...);
      };
    }
