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

#include <seda/Strategy.hpp>

#include <sdpa/capability.hpp>
#include <sdpa/sdpa-config.hpp>
#include <sdpa/util/Config.hpp>
#include <sdpa/daemon/SchedulerImpl.hpp>
#include <sdpa/daemon/JobManager.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/BackupService.hpp>

#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/ConfigNokEvent.hpp>
#include <sdpa/events/ConfigOkEvent.hpp>
#include <sdpa/events/ConfigReplyEvent.hpp>
#include <sdpa/events/ConfigRequestEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>
#include <sdpa/events/InterruptEvent.hpp>
#include <sdpa/events/JobFailedAckEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/JobFinishedAckEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>
#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/StartUpEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/SubscribeEvent.hpp>
#include <sdpa/events/WorkerRegistrationAckEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>

#include <sdpa/types.hpp>

#include <we/type/schedule_data.hpp>
#include <we/type/user_data.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/weak_ptr.hpp>
#include <boost/serialization/set.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/utility.hpp>
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
                    unsigned int cap = 10000,
                    unsigned int rank = 0
                   , const std::string& guiUrl = ""
                   );

      SDPA_DECLARE_LOGGER();

      const std::string& name() const { return Strategy::name(); }
      const unsigned int& rank() const { return m_nRank; }
      unsigned int& rank() { return m_nRank; }
      virtual const std::string url() const {return std::string();}
      const unsigned int& capacity() const { return m_nCap; }
      unsigned int& capacity() { return m_nCap; }
      const sdpa::worker_id_t& agent_uuid() { return m_strAgentUID; }

      void start_agent( bool bUseReqModel, const bfs::path& bkpFile); // from cfg file!
      void start_agent( bool bUseReqModel, std::string& strBackup);
      void start_agent( bool bUseReqModel = true); // no recovery

      std::string last_backup() const;
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
      virtual bool cancelled(const id_type& id);

      void addCapability(const capability_t& cpb);
      void getCapabilities(sdpa::capabilities_set_t& cpbset);

      NotificationService* gui_service() { return &m_guiService; }

      virtual void handleInterruptEvent() = 0;

    protected:

      // stages
      void setStage(const seda::Stage::Ptr& stage){ ptr_daemon_stage_ = stage; }
      virtual seda::Stage::Ptr to_master_stage() const { return ptr_to_master_stage_ ; }
      virtual seda::Stage::Ptr to_slave_stage() const { return ptr_to_slave_stage_ ; }

      // masters and subscribers
      sdpa::master_info_list_t& getListMasterInfo() { return m_arrMasterInfo; }

      template <typename T>
      void notifyMasters(const T&);

      void unsubscribe(const sdpa::agent_id_t&);
      void subscribe(const sdpa::agent_id_t&, const sdpa::job_id_list_t&);
      bool isSubscriber(const sdpa::agent_id_t&);
      bool subscribedFor(const sdpa::agent_id_t&, const sdpa::job_id_t&);

      // configuration
      sdpa::util::Config& cfg() { return daemon_cfg_;}
      void setDefaultConfiguration();
      virtual void configure_network( const std::string& daemonUrl/*, const std::string& masterName = ""*/ );
      virtual void shutdown_network();

      // agent info and properties
      bool isStarted(){ return m_bStarted;  }
      void setStarted(bool bVal = true){ m_bStarted = bVal; }

      bool isConfigured(){ return m_bConfigOk; }
      void setConfigured(bool bVal = true) { m_bConfigOk = bVal; }

      void setStopped(bool bVal = true) { m_bStopped = bVal; }

      void setRequestsAllowed(bool bVal = true) { m_bRequestsAllowed = bVal; }
      virtual void updateLastRequestTime();
      virtual bool requestsAllowed();
      void interrupt();

      bool isOwnCapability(const sdpa::capability_t& cpb)
      {
    	  return (cpb.owner()==name());
      }

      virtual void print()
      {
    	  SDPA_LOG_DEBUG("The content of the JobManager is:");
    	  jobManager()->print();

    	  SDPA_LOG_DEBUG("The content of the Scheduler is:");
    	  scheduler()->print();
      }

      // event handlers
      virtual void perform(const seda::IEvent::Ptr&);
      virtual void handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent*);
      virtual void handleConfigReplyEvent(const sdpa::events::ConfigReplyEvent*);
      virtual void handleSubmitJobAckEvent(const sdpa::events::SubmitJobAckEvent* );
      virtual void handleCancelJobEvent(const sdpa::events::CancelJobEvent*);
      virtual void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent* );
      virtual void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
      virtual void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );
      virtual void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent* );
      virtual void handleJobFailedAckEvent(const sdpa::events::JobFailedAckEvent* );
      virtual void handleQueryJobStatusEvent(const sdpa::events::QueryJobStatusEvent* );
      virtual void handleRetrieveJobResultsEvent(const sdpa::events::RetrieveJobResultsEvent* );
      virtual void handleCapabilitiesGainedEvent(const sdpa::events::CapabilitiesGainedEvent*);
      virtual void handleCapabilitiesLostEvent(const sdpa::events::CapabilitiesLostEvent*);
      virtual void handleSubscribeEvent( const sdpa::events::SubscribeEvent* pEvt );

      // agent fsm (actions)
      virtual void action_configure( const sdpa::events::StartUpEvent& );
      virtual void action_config_ok( const sdpa::events::ConfigOkEvent& );
      virtual void action_config_nok( const sdpa::events::ConfigNokEvent& );
      virtual void action_delete_job( const sdpa::events::DeleteJobEvent& );
      virtual void action_request_job( const sdpa::events::RequestJobEvent& );
      virtual void action_submit_job( const sdpa::events::SubmitJobEvent& );
      virtual void action_config_request( const sdpa::events::ConfigRequestEvent& );
      virtual void action_register_worker(const sdpa::events::WorkerRegistrationEvent& );
      virtual void action_error_event(const sdpa::events::ErrorEvent& );

      // event communication
      virtual void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& e);
      virtual void sendEventToMaster(const sdpa::events::SDPAEvent::Ptr& e); // 0 retries, 1 second timeout
      virtual void sendEventToSlave(const sdpa::events::SDPAEvent::Ptr& e); // 0 retries, 1 second timeout

      // registration
      virtual void requestRegistration();
      virtual void requestRegistration(const MasterInfo& masterInfo);
      virtual void registerWorker(const sdpa::events::WorkerRegistrationEvent& evtRegWorker);

      // workflow engine
      virtual we::mgmt::basic_layer* workflowEngine() const { return ptr_workflow_engine_; }
      virtual bool hasWorkflowEngine() { return ptr_workflow_engine_;}

      template <typename T>
        void createWorkflowEngine()
      {
    	  ptr_workflow_engine_ = new T(this, boost::bind(&GenericDaemon::gen_id, this));
      }

      // workflow engine notifications
      virtual void activityFailed( const Worker::worker_id_t& worker_id
                                  , const job_id_t& jobId
                                  , const std::string& result
                                  , const int error_code
                                  , const std::string& reason
      );

      virtual void activityFinished(const Worker::worker_id_t& worker_id, const job_id_t & id, const result_type& result );
      virtual void activityCancelled(const Worker::worker_id_t& worker_id, const job_id_t& id);
      virtual void submitWorkflow(const id_type& id, const encoded_type& );
      virtual void cancelWorkflow(const id_type& workflowId, const std::string& reason);

      // workers
      virtual Worker::worker_id_t getWorkerId(unsigned int rank);
      virtual const Worker::worker_id_t& findWorker(const sdpa::job_id_t& job_id) const;
      const Worker::ptr_t & findWorker(const Worker::worker_id_t& worker_id) const;
      void getWorkerCapabilities(const Worker::worker_id_t&, sdpa::capabilities_set_t&);
      virtual void serveJob(const Worker::worker_id_t& worker_id, const job_id_t& jobId );
      virtual void requestJob(const MasterInfo& masterInfo);
      virtual void addWorker( const Worker::worker_id_t& workerId,
                              unsigned int cap,
                              const capabilities_set_t& cpbset,
                              const unsigned int& rank = 0,
                              const sdpa::worker_id_t& agent_uuid  = "");

      void eworknotreg();

      // jobs
      Job::ptr_t& findJob(const sdpa::job_id_t& job_id ) const;
      void deleteJob(const sdpa::job_id_t& );
      std::string gen_id() { return sdpa::JobId ().str (); }
      const job_requirements_t getJobRequirements(const sdpa::job_id_t& jobId) const;

    public:
      // scheduler
      Scheduler::ptr_t scheduler() const {return ptr_scheduler_;}
      JobManager::ptr_t jobManager() const { return ptr_job_man_; }
    protected:
      virtual void createScheduler(bool bUseReqModel) = 0;
      virtual void schedule(const sdpa::job_id_t& job);
      virtual void reschedule(const sdpa::job_id_t& job);
      virtual bool isScheduled(const sdpa::job_id_t& job_id) { return scheduler()->has_job(job_id); }
      void reScheduleAllMasterJobs();

      // backup
      friend class boost::serialization::access;

      template <class Archive>
      void serialize(Archive& ar, const unsigned int)
      {
    	  ar & ptr_job_man_;
    	  ar & ptr_scheduler_;
    	  ar & ptr_workflow_engine_;
    	  ar & m_arrMasterInfo;
    	  ar & m_listSubscribers;
      }

      void backupJobManager(boost::archive::text_oarchive& oa) { oa << ptr_job_man_;}
      void recoverJobManager(boost::archive::text_iarchive& ia) { ia >> ptr_job_man_; }

      void backupScheduler(boost::archive::text_oarchive& oa) { oa << ptr_scheduler_;}
      void recoverScheduler(boost::archive::text_iarchive& ia) { ia >> ptr_scheduler_;}

      void backup( std::ostream& );
      void recover( std::istream& );

      // data members
    protected:
      mutex_type mtx_;
      condition_type cond_can_start_;

      sdpa::master_info_list_t m_arrMasterInfo;
      sdpa::subscriber_map_t m_listSubscribers;

    private:
      sdpa::weak_ptr<seda::Stage> ptr_daemon_stage_;
      std::string m_to_master_stage_name_;
      std::string m_to_slave_stage_name_;
      seda::Stage::Ptr ptr_to_master_stage_;
      seda::Stage::Ptr ptr_to_slave_stage_;
      sdpa::util::Config daemon_cfg_;

    protected:
      JobManager::ptr_t ptr_job_man_;
      Scheduler::ptr_t ptr_scheduler_;
      we::mgmt::basic_layer* ptr_workflow_engine_;

    private:

      unsigned int m_nRank;
      unsigned int m_nCap; // maximum number of external jobs
      sdpa::worker_id_t m_strAgentUID;
      unsigned int m_nExternalJobs;
      sdpa::util::time_type m_ullPollingInterval;

      bool m_bRequestsAllowed;
      bool m_bStarted;
      bool m_bConfigOk;
      bool m_bStopped;
      mutex_type mtx_subscriber_;
      mutex_type mtx_master_;
      mutex_type mtx_cpb_;

      BackupService m_threadBkpService;
      sdpa::capabilities_set_t m_capabilities;
      sdpa::util::time_type m_last_request_time;
      NotificationService m_guiService;
    };

     /**
     * Send a notification of type T to the masters
     * @param[in] ptrNotEvt: Event to be sent to the master
     */
    template <typename T>
    void GenericDaemon::notifyMasters(const T& ptrNotEvt)
    {
      lock_type lock(mtx_master_);
      if(m_arrMasterInfo.empty())
      {
        SDPA_LOG_INFO("The master list is empty. No master to be notified exist!");
        return;
      }

      BOOST_FOREACH(sdpa::MasterInfo & masterInfo, m_arrMasterInfo)
      {
        if( masterInfo.is_registered() )
        {
          ptrNotEvt->to() = masterInfo.name();
          SDPA_LOG_INFO("Send notification to the master "<<masterInfo.name());
          sendEventToMaster(ptrNotEvt);
        }
      }
    }
  }
}

//BOOST_SERIALIZATION_ASSUME_ABSTRACT( sdpa::daemon::GenericDaemon )

#endif
