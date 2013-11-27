/*
 * =====================================================================================
 *
 *       Filename:  GenericDaemon.hpp
 *
 *    Description:  Generic daemon header file
 *
 *        Version:  1.0
 *        Created:  2009
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef SDPA_DAEMON_GENERIC_DAEMON_HPP
#define SDPA_DAEMON_GENERIC_DAEMON_HPP 1

#include <fhg/assert.hpp>

#include <seda/Stage.hpp>
#include <seda/Strategy.hpp>

#include <sdpa/capability.hpp>
#include <sdpa/logging.hpp>
#include <sdpa/daemon/scheduler/SchedulerBase.hpp>
#include <sdpa/daemon/JobManager.hpp>

#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>
#include <sdpa/events/JobFailedAckEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/JobFinishedAckEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/SubscribeEvent.hpp>
#include <sdpa/events/WorkerRegistrationAckEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>

#include <sdpa/types.hpp>

#include <we/type/schedule_data.hpp>
#include <we/type/user_data.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>


#include <sdpa/daemon/NotificationService.hpp>

namespace sdpa {
  namespace daemon {
    class GenericDaemon : public sdpa::daemon::IAgent,
                          public seda::Strategy,
                          public sdpa::events::EventHandler,
                          boost::noncopyable
    {
    public:
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;
      typedef boost::condition_variable_any condition_type;
      typedef sdpa::shared_ptr<GenericDaemon> ptr_t;

      GenericDaemon(const std::string name = "orchestrator_0",
                    const sdpa::master_info_list_t m_arrMasterInfo =  sdpa::master_info_list_t(),
                    unsigned int rank = 0
                   , const boost::optional<std::string>& guiUrl = boost::none
                   , bool create_wfe = false
                   );
      virtual ~GenericDaemon() {}

      SDPA_DECLARE_LOGGER();

      const std::string& name() const { return Strategy::name(); }
      const unsigned int& rank() const { return m_nRank; }
      unsigned int& rank() { return m_nRank; }
      virtual const std::string url() const {return std::string();}
      const sdpa::worker_id_t& agent_uuid() { return m_strAgentUID; }

      void start_agent();

      void shutdown();

      void addMaster(const agent_id_t& );
      void addMasters(const agent_id_list_t& );
      void removeMaster(const agent_id_t& masterId);
      void removeMasters(const agent_id_list_t& );
      size_t numberOfMasterAgents() { return m_arrMasterInfo.size(); }

      bool isTop() { return false; }

      // WE interface
      virtual void submit( const id_type & id
                         , const encoded_type&
                         , const requirement_list_t& = requirement_list_t()
                         , const we::type::schedule_data& = we::type::schedule_data()
                         , const we::type::user_data & = we::type::user_data ()
                         );
      virtual bool cancel(const id_type & id, const reason_type& reason);
      virtual bool finished(const id_type & id, const result_type& result);
      virtual bool failed( const id_type& wfId, const result_type& res, int errc, std::string const& reason);
      virtual bool canceled(const id_type& id);

      void addCapability(const capability_t& cpb);
      void getCapabilities(sdpa::capabilities_set_t& cpbset);

      virtual void handleWorkerRegistrationEvent(const sdpa::events::WorkerRegistrationEvent* );
      virtual void handleDeleteJobEvent(const sdpa::events::DeleteJobEvent* );
      virtual void handleSubmitJobEvent(const sdpa::events::SubmitJobEvent* );
      virtual void handleErrorEvent(const sdpa::events::ErrorEvent* );

      SchedulerBase::ptr_t scheduler() const {return ptr_scheduler_;}
    protected:

      // stages
      void setStage(const seda::Stage::Ptr& stage){ ptr_daemon_stage_ = stage; }
      virtual seda::Stage::Ptr to_master_stage() const { return ptr_to_master_stage_ ; }
      virtual seda::Stage::Ptr to_slave_stage() const { return ptr_to_slave_stage_ ; }
      virtual sdpa::weak_ptr<seda::Stage> daemon_stage() const { return ptr_daemon_stage_ ; }

      // masters and subscribers
      sdpa::master_info_list_t& getListMasterInfo() { return m_arrMasterInfo; }

      void unsubscribe(const sdpa::agent_id_t&);
      void subscribe(const sdpa::agent_id_t&, const sdpa::job_id_list_t&);
      bool isSubscriber(const sdpa::agent_id_t&);
      bool subscribedFor(const sdpa::agent_id_t&, const sdpa::job_id_t&);

      // agent info and properties

      bool isOwnCapability(const sdpa::capability_t& cpb)
      {
    	  return (cpb.owner()==name());
      }

      // event handlers
      virtual void perform(const seda::IEvent::Ptr&);
      virtual void handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent*);
      virtual void handleSubmitJobAckEvent(const sdpa::events::SubmitJobAckEvent* );
      virtual void handleCancelJobEvent(const sdpa::events::CancelJobEvent*) = 0;
      virtual void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent* ) = 0;
      virtual void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* ) = 0;
      virtual void handleJobFailedEvent(const sdpa::events::JobFailedEvent* ) = 0;
      virtual void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent* );
      virtual void handleJobFailedAckEvent(const sdpa::events::JobFailedAckEvent* );
      virtual void handleQueryJobStatusEvent(const sdpa::events::QueryJobStatusEvent* );
      virtual void handleRetrieveJobResultsEvent(const sdpa::events::RetrieveJobResultsEvent* );
      virtual void handleCapabilitiesGainedEvent(const sdpa::events::CapabilitiesGainedEvent*);
      virtual void handleCapabilitiesLostEvent(const sdpa::events::CapabilitiesLostEvent*);
      virtual void handleSubscribeEvent( const sdpa::events::SubscribeEvent* pEvt );
      virtual void handleJobStalledEvent (const sdpa::events::JobStalledEvent *);
      virtual void handleJobRunningEvent (const sdpa::events::JobRunningEvent *);

      // event communication
      virtual void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& e);
      virtual void sendEventToMaster(const sdpa::events::SDPAEvent::Ptr& e); // 0 retries, 1 second timeout
      virtual void sendEventToSlave(const sdpa::events::SDPAEvent::Ptr& e); // 0 retries, 1 second timeout

      // registration
      virtual void requestRegistration(const MasterInfo& masterInfo);
      virtual void registerWorker(const sdpa::events::WorkerRegistrationEvent& evtRegWorker);

      // workflow engine
      virtual we::mgmt::layer* workflowEngine() const { return ptr_workflow_engine_; }
      virtual bool hasWorkflowEngine() const { return ptr_workflow_engine_;}

      // workflow engine notifications
      virtual void submitWorkflow(const job_id_t& id);

      // workers
      void serveJob(const Worker::worker_id_t&, const job_id_t&);
      void serveJob(const sdpa::worker_id_list_t& worker_list, const job_id_t& jobId);

      // jobs
      std::string gen_id() { return sdpa::JobId ().str (); }

    public:
      // forwarding to jobManager() only:
      void addJob( const sdpa::job_id_t& jid, const Job::ptr_t& pJob, const job_requirements_t& reqList = job_requirements_t())
      {
        return jobManager().addJob(jid, pJob, reqList);
      }
      bool hasJobs()
      {
        return jobManager().hasJobs();
      }
      Job::ptr_t findJob(const sdpa::job_id_t& job_id ) const
      {
        return jobManager().findJob(job_id);
      }
      void deleteJob(const sdpa::job_id_t& jobId)
      {
        jobManager().deleteJob(jobId);
      }
      const job_requirements_t getJobRequirements(const sdpa::job_id_t& jobId) const
      {
        return jobManager().getJobRequirements(jobId);
      }

      // forwaring to scheduler() only:
      Worker::ptr_t const & findWorker(const Worker::worker_id_t& worker_id ) const
      {
        return scheduler()->findWorker(worker_id);
      }
      void getWorkerCapabilities(const Worker::worker_id_t& worker_id, sdpa::capabilities_set_t& wCpbset)
      {
        scheduler()->getWorkerCapabilities(worker_id, wCpbset);
      }

    protected:
      const JobManager& jobManager() const { return _job_manager; }
      JobManager& jobManager() { return _job_manager; }
      //! \todo type of Scheduler should be template parameter
      virtual void createScheduler() = 0;

      // data members
    protected:
      mutex_type mtx_;

      sdpa::master_info_list_t m_arrMasterInfo;
      sdpa::subscriber_map_t m_listSubscribers;

    private:
      sdpa::weak_ptr<seda::Stage> ptr_daemon_stage_;
      std::string m_to_master_stage_name_;
      std::string m_to_slave_stage_name_;
      seda::Stage::Ptr ptr_to_master_stage_;
      seda::Stage::Ptr ptr_to_slave_stage_;

    protected:
      JobManager _job_manager;
      SchedulerBase::ptr_t ptr_scheduler_;
      we::mgmt::layer* ptr_workflow_engine_;

    private:

      unsigned int m_nRank;
      sdpa::worker_id_t m_strAgentUID;

      mutex_type mtx_subscriber_;
      mutex_type mtx_master_;
      mutex_type mtx_cpb_;

      sdpa::capabilities_set_t m_capabilities;

    protected:
      boost::optional<NotificationService> m_guiService;

    private:
      std::vector<std::string> _stages_to_remove;

      unsigned int _max_consecutive_registration_attempts;
      unsigned int _max_consecutive_network_faults;
      boost::posix_time::time_duration _registration_timeout;
    };
  }
}

#endif
