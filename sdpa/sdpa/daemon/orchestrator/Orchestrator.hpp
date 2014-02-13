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
#include <sdpa/daemon/scheduler/SimpleScheduler.hpp>

namespace sdpa {
  namespace daemon {
    class Orchestrator : public GenericDaemon
    {
      public:
      Orchestrator (const std::string &name, const std::string& url, std::string kvs_host, std::string kvs_port)
      : GenericDaemon ( name, url, kvs_host, kvs_port, sdpa::master_info_list_t() /*, NULL*/)
      {
        ptr_scheduler_ = SchedulerBase::ptr_t (new SimpleScheduler (this));
        ptr_scheduler_->start_threads(); //! \note: can't do in ctor: vtable not set up yet

        if (!isTop())
        {
          lock_type lock (mtx_master_);
          BOOST_FOREACH (sdpa::MasterInfo& masterInfo, m_arrMasterInfo)
          {
            requestRegistration (masterInfo);
          }
        }
      }

      virtual void handleJobFinishedEvent( const sdpa::events::JobFinishedEvent* );
      virtual void handleJobFailedEvent( const sdpa::events::JobFailedEvent* );

      virtual void handleCancelJobEvent( const sdpa::events::CancelJobEvent* pEvt );
      virtual void handleCancelJobAckEvent( const sdpa::events::CancelJobAckEvent* pEvt );
      virtual void handleDeleteJobEvent(const sdpa::events::DeleteJobEvent* );
      virtual void handleRetrieveJobResultsEvent(const sdpa::events::RetrieveJobResultsEvent* );
      virtual void handleQueryJobStatusEvent(const sdpa::events::QueryJobStatusEvent* );
      virtual void handleDiscoverJobStatesEvent (const sdpa::events::DiscoverJobStatesEvent *pEvt);
      virtual void handleDiscoverJobStatesReplyEvent (const sdpa::events::DiscoverJobStatesReplyEvent *pEvt);

      template <typename T>
      void notifySubscribers(const T& ptrEvt);
    };
  }
}

#endif //SDPA_ORCHESTRATORTOR_HPP
