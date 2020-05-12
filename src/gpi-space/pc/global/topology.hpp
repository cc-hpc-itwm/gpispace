#pragma once

#include <gpi-space/pc/global/itopology.hpp>
#include <gpi-space/pc/memory/manager.hpp>

#include <rpc/function_description.hpp>
#include <rpc/remote_tcp_endpoint.hpp>
#include <rpc/service_tcp_provider.hpp>
#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <vmem/gaspi_context.hpp>

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
                   , fhg::vmem::gaspi_context&
                   , std::unique_ptr<fhg::rpc::service_tcp_provider_with_deferred_dispatcher>
                   );


        virtual bool is_master () const override;

        virtual void alloc ( const gpi::pc::type::segment_id_t segment
                           , const gpi::pc::type::handle_t
                           , const gpi::pc::type::offset_t
                           , const gpi::pc::type::size_t size
                           , const gpi::pc::type::size_t local_size
                           , const std::string & name
                           ) override;

        virtual void free (const gpi::pc::type::handle_t) override;

        virtual void add_memory ( const gpi::pc::type::segment_id_t seg_id
                                , const std::string & url
                                ) override;
        virtual void del_memory (const gpi::pc::type::segment_id_t seg_id) override;

      private:
        mutable std::mutex m_global_alloc_mutex;

        fhg::vmem::gaspi_context& _gaspi_context;

        FHG_RPC_FUNCTION_DESCRIPTION ( alloc_desc
                                     , void ( type::segment_id_t
                                            , type::handle_t
                                            , type::offset_t
                                            , type::size_t size
                                            , type::size_t local_size
                                            , std::string name
                                            )
                                     );
        FHG_RPC_FUNCTION_DESCRIPTION ( free_desc
                                     , void (type::handle_t)
                                     );

        FHG_RPC_FUNCTION_DESCRIPTION ( add_memory_desc
                                     , void ( type::segment_id_t
                                            , std::string
                                            )
                                     );
        FHG_RPC_FUNCTION_DESCRIPTION ( del_memory_desc
                                     , void (type::segment_id_t)
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
