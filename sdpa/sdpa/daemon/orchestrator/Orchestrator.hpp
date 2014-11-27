/*
 * =====================================================================================
 *
 *       Filename:  Orchestrator.hpp
 *
 *    Description:  Contains the Orchestrator class
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef SDPA_ORCHESTRATORTOR_HPP
#define SDPA_ORCHESTRATORTOR_HPP 1

#include <sdpa/daemon/GenericDaemon.hpp>

#include <network/server.hpp>

#include <rpc/server.hpp>

#include <boost/asio/ip/tcp.hpp>

namespace sdpa {
  namespace daemon {
    class Orchestrator : public GenericDaemon
    {
      public:
      Orchestrator ( const std::string &name
                   , const std::string& url
                   , boost::asio::io_service& peer_io_service
                   , boost::asio::io_service& kvs_client_io_service
                   , std::string kvs_host, std::string kvs_port
                   , boost::asio::io_service& rpc_io_service
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
      std::vector<std::unique_ptr<fhg::network::connection_type>> _rpc_connections;
      fhg::rpc::service_dispatcher _rpc_dispatcher;
      fhg::network::continous_acceptor<boost::asio::ip::tcp> _rpc_acceptor;
    };
  }
}

#endif //SDPA_ORCHESTRATORTOR_HPP
