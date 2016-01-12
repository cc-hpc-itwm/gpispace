#pragma once

#include <sdpa/daemon/GenericDaemon.hpp>

#include <rpc/server_with_multiple_clients.hpp>

#include <boost/asio/ip/tcp.hpp>

namespace sdpa {
  namespace daemon {
    class Orchestrator final : public GenericDaemon
    {
      public:
      Orchestrator ( const std::string &name
                   , const std::string& url
                   , std::unique_ptr<boost::asio::io_service> peer_io_service
                   , fhg::log::Logger&
                   );

      virtual void handleJobFinishedEvent
        (fhg::com::p2p::address_t const& source, const sdpa::events::JobFinishedEvent* ) override;
      virtual void handleJobFailedEvent
        (fhg::com::p2p::address_t const& source, const sdpa::events::JobFailedEvent* ) override;

      virtual void handleCancelJobEvent
        (fhg::com::p2p::address_t const& source, const sdpa::events::CancelJobEvent* pEvt ) override;
      virtual void handleCancelJobAckEvent
        (fhg::com::p2p::address_t const& source, const sdpa::events::CancelJobAckEvent* pEvt ) override;
      virtual void handleDeleteJobEvent
        (fhg::com::p2p::address_t const& source, const sdpa::events::DeleteJobEvent* ) override;

      boost::asio::ip::tcp::endpoint rpc_local_endpoint() const;

    private:
      fhg::rpc::service_dispatcher _rpc_dispatcher;
      fhg::rpc::server_with_multiple_clients _rpc_server;

      boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
        _event_handler_thread;
    };
  }
}
