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
                   );

      virtual void handleJobFinishedEvent( const sdpa::events::JobFinishedEvent* ) override;
      virtual void handleJobFailedEvent( const sdpa::events::JobFailedEvent* ) override;

      virtual void handleCancelJobEvent( const sdpa::events::CancelJobEvent* pEvt ) override;
      virtual void handleCancelJobAckEvent( const sdpa::events::CancelJobAckEvent* pEvt ) override;
      virtual void handleDeleteJobEvent(const sdpa::events::DeleteJobEvent* ) override;

    private:
      std::list<agent_id_t> subscribers (job_id_t job_id) const;
    };
  }
}

#endif //SDPA_ORCHESTRATORTOR_HPP
