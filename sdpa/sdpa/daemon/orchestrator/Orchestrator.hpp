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
      SDPA_DECLARE_LOGGER();

      Orchestrator (const std::string &name, const std::string& url)
      : GenericDaemon ( name, url, sdpa::master_info_list_t() /*, NULL*/),
        SDPA_INIT_LOGGER(name)
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

      template <typename T>
      void notifySubscribers(const T& ptrEvt);

      virtual void pause(const job_id_t& id );
      virtual void resume(const job_id_t& id );
    };
  }
}

#endif //SDPA_ORCHESTRATORTOR_HPP
