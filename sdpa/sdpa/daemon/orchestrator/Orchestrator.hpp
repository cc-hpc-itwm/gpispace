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
#include <sdpa/daemon/SchedulerImpl.hpp>

namespace sdpa {
  namespace daemon {
    class Orchestrator : public sdpa::fsm::bmsm::DaemonFSM
    {
      public:
      typedef sdpa::shared_ptr<Orchestrator> ptr_t;
      SDPA_DECLARE_LOGGER();

      Orchestrator( const std::string &name = ""
                    , const std::string& url = "")
      : DaemonFSM( name, sdpa::master_info_list_t() /*, NULL*/),
        SDPA_INIT_LOGGER(name),
        url_(url)
      {}

			static Orchestrator::ptr_t create
        (const std::string& name, const std::string& url);

      void handleJobFinishedEvent( const sdpa::events::JobFinishedEvent* );
      void handleJobFailedEvent( const sdpa::events::JobFailedEvent* );

      void handleCancelJobEvent( const sdpa::events::CancelJobEvent* pEvt );
      void handleCancelJobAckEvent( const sdpa::events::CancelJobAckEvent* pEvt );

      const std::string url() const {return url_;}
      bool isTop() { return true; }

      void cancelPendingJob(const sdpa::events::CancelJobEvent& evt);

      template <typename T>
      void notifySubscribers(const T& ptrEvt);

      private:
      std::string url_;
	  };
	}
}

#endif //SDPA_ORCHESTRATORTOR_HPP
