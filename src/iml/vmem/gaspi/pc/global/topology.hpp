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

#include <iml/AllocationHandle.hpp>
#include <iml/MemoryOffset.hpp>
#include <iml/MemorySize.hpp>
#include <iml/SegmentDescription.hpp>
#include <iml/SegmentHandle.hpp>
#include <iml/vmem/gaspi/pc/global/itopology.hpp>
#include <iml/vmem/gaspi/pc/memory/manager.hpp>
#include <iml/vmem/gaspi_context.hpp>

#include <rpc/function_description.hpp>
#include <rpc/remote_tcp_endpoint.hpp>
#include <rpc/service_tcp_provider.hpp>
#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <boost/noncopyable.hpp>

#include <memory>
#include <mutex>
#include <list>

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      class topology_t : boost::noncopyable
                       , public itopology_t
      {
      public:
        topology_t ( memory::manager_t& memory_manager
                   , fhg::iml::vmem::gaspi_context&
                   , std::unique_ptr<fhg::rpc::service_tcp_provider_with_deferred_dispatcher>
                   );


        virtual bool is_master () const override;

        virtual void alloc ( const iml::SegmentHandle segment
                           , const iml::AllocationHandle
                           , const iml::MemoryOffset
                           , const iml::MemorySize size
                           , const iml::MemorySize local_size
                           ) override;

        virtual void free (const iml::AllocationHandle) override;

        virtual void add_memory ( const iml::SegmentHandle seg_id
                                , iml::SegmentDescription const& description
                                , unsigned long total_size
                                ) override;
        virtual void del_memory (const iml::SegmentHandle seg_id) override;

      private:
        mutable std::mutex m_global_alloc_mutex;

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
