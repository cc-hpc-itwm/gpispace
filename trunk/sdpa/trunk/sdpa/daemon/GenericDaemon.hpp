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

#include <seda/Strategy.hpp>

//#include <seda/comm/delivery_service.hpp>
//#include <seda/comm/ServiceThread.hpp>

#include <sdpa/sdpa-config.hpp>
#include <sdpa/util/Config.hpp>
#include <sdpa/daemon/IAgent.hpp>
#include <sdpa/daemon/SchedulerImpl.hpp>
#include <sdpa/daemon/JobManager.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/GenericDaemonActions.hpp>
#include <sdpa/daemon/BackupService.hpp>

#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/WorkerRegistrationAckEvent.hpp>
#include <sdpa/events/ConfigReplyEvent.hpp>
#include <sdpa/events/EventHandler.hpp>
#include <sdpa/types.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/weak_ptr.hpp>
#include <boost/utility.hpp>

namespace sdpa { namespace tests { class DaemonFSMTest_SMC; class DaemonFSMTest_BSC;}}

namespace sdpa { namespace daemon {

  class GenericDaemon : public sdpa::daemon::GenericDaemonActions,
						public sdpa::daemon::IComm,
						public seda::Strategy,
						public sdpa::events::EventHandler,
						public IAgent,
						boost::noncopyable
{
  public:
	  typedef boost::recursive_mutex mutex_type;
	  typedef boost::unique_lock<mutex_type> lock_type;
	  typedef boost::condition_variable_any condition_type;

	  typedef sdpa::shared_ptr<GenericDaemon> ptr_t;

	  GenericDaemon( const std::string name = "orchestrator_0", IWorkflowEngine* pArgSdpa2Gwes = NULL );
	  virtual ~GenericDaemon();

	  SDPA_DECLARE_LOGGER();

	  // API

	  virtual void start_fsm();

	  void start_agent( const bfs::path& bkpFile, const std::string& cfgFile = ""  ); // from cfg file!
	  void start_agent( std::string& strBackup, const std::string& cfgFile = ""  );
	  void start_agent( const std::string& cfgFile = "" ); // no recovery

	  void shutdown(std::string&); // no backup
	  void shutdown( const bfs::path& backup_path );
	  void shutdown( );

	  virtual void configure_network( const std::string& daemonUrl, const std::string& masterName = "" );
	  virtual void shutdown_network();
	  void stop();

	  virtual void perform(const seda::IEvent::Ptr&);
	  virtual void schedule(const sdpa::job_id_t& job);

	  bool is_configured() { return m_bConfigOk; }
	  bool is_stopped() { return m_bStopped; }
	  bool is_started() { return m_bStarted; }

	  void setDefaultConfiguration();

	  // daemon actions
	  virtual void action_configure( const sdpa::events::StartUpEvent& );
	  virtual void action_config_ok( const sdpa::events::ConfigOkEvent& );
	  virtual void action_config_nok( const sdpa::events::ConfigNokEvent& );
	  virtual void action_interrupt( const sdpa::events::InterruptEvent& );
	  //virtual void action_lifesign( const sdpa::events::LifeSignEvent& );
	  virtual void action_delete_job( const sdpa::events::DeleteJobEvent& );
	  virtual void action_request_job( const sdpa::events::RequestJobEvent& );
	  virtual void action_submit_job( const sdpa::events::SubmitJobEvent& );
	  virtual void action_config_request( const sdpa::events::ConfigRequestEvent& );
	  virtual void action_register_worker(const sdpa::events::WorkerRegistrationEvent& );
	  virtual void action_error_event(const sdpa::events::ErrorEvent& );

	  virtual void handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent*);
	  virtual void handleConfigReplyEvent(const sdpa::events::ConfigReplyEvent*);

	  // job event handlers
	  virtual void handleSubmitJobAckEvent(const sdpa::events::SubmitJobAckEvent* );
	  virtual void handleCancelJobEvent(const sdpa::events::CancelJobEvent*);
	  virtual void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent* );
	  virtual void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
	  virtual void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );
	  virtual void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent* );
	  virtual void handleJobFailedAckEvent(const sdpa::events::JobFailedAckEvent* );
	  virtual void handleQueryJobStatusEvent(const sdpa::events::QueryJobStatusEvent* );
	  virtual void handleRetrieveJobResultsEvent(const sdpa::events::RetrieveJobResultsEvent* );

	  virtual void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& e);
	  virtual void sendEventToMaster(const sdpa::events::SDPAEvent::Ptr& e); // 0 retries, 1 second timeout
	  virtual void sendEventToSlave(const sdpa::events::SDPAEvent::Ptr& e); // 0 retries, 1 second timeout
	  virtual void requestRegistration();
	  virtual void requestJob();

	  virtual sdpa::status_t getStatus() { throw std::runtime_error("not implemented by the generic daemon!"); }

	  virtual bool is_scheduled(const sdpa::job_id_t& job_id) { return ptr_scheduler_->has_job(job_id); }

      // WE interface
	  virtual void submit(const id_type & id, const encoded_type & );
	  virtual bool cancel(const id_type & id, const reason_type & reason);
	  virtual bool finished(const id_type & id, const result_type & result);
	  virtual bool failed(const id_type & id, const result_type & result);
	  virtual bool cancelled(const id_type & id);

	  virtual void workerJobFailed(const Worker::worker_id_t& worker_id, const job_id_t&, const std::string& result /*or reason*/) ;
	  virtual void workerJobFinished(const Worker::worker_id_t& worker_id, const job_id_t & id, const result_type& result );
	  virtual void workerJobCancelled(const Worker::worker_id_t& worker_id, const job_id_t& id);

	  virtual void submitWorkflow(const id_type& id, const encoded_type& );
	  virtual void cancelWorkflow(const id_type& workflowId, const std::string& reason);

	  virtual void activityCancelled(const id_type& id, const std::string& data);

	  virtual const Worker::worker_id_t& findWorker(const sdpa::job_id_t& job_id) const;

	  const Worker::ptr_t & findWorker(const Worker::worker_id_t& worker_id) const;
	  virtual void addWorker( const Worker::worker_id_t& workerId, unsigned int rank, const sdpa::worker_id_t& agent_uuid  = "");

	  std::string master()const { return master_;}
	  void setMaster( std::string masterName ){ master_=masterName;}

	  const std::string& name() const { return Strategy::name(); }

	  JobManager::ptr_t jobManager() const { return ptr_job_man_; }

	  Job::ptr_t& findJob(const sdpa::job_id_t& job_id ) const;
	  void deleteJob(const sdpa::job_id_t& );

	  void setStage(const seda::Stage::Ptr& stage)
	  {
		  ptr_daemon_stage_ = stage ;
	  }

	  virtual seda::Stage::Ptr to_master_stage() const { return ptr_to_master_stage_ ; }
	  virtual seda::Stage::Ptr to_slave_stage() const { return ptr_to_slave_stage_ ; }

	  //virtual void set_to_slave_stage(seda::Stage* argStage) { ptr_to_slave_stage_= argStage; }

	  virtual bool is_registered() const { return m_bRegistered;}

	  sdpa::util::Config& cfg() { return daemon_cfg_;}

	  const unsigned int& rank() const { return m_nRank; }
	  unsigned int& rank() { return m_nRank; }
	  const sdpa::worker_id_t& agent_uuid() { return m_strAgentUID; }

	  //boost::bind(&sdpa_daemon::gen_id, this))
	  std::string gen_id() { JobId jobId; return jobId.str(); }

	  void jobFailed(const job_id_t&, const std::string& reason);
	  const preference_t& getJobPreferences(const sdpa::job_id_t& jobId) const;

	  virtual bool requestsAllowed(const sdpa::util::time_type&);

	  Scheduler::ptr_t scheduler() const {return ptr_scheduler_;}
	  void  setScheduler(Scheduler* p) {ptr_scheduler_ = Scheduler::ptr_t(p);}

	  template <class Archive>
	  void serialize(Archive& ar, const unsigned int)
	  {
		  ar & ptr_job_man_;
		  ar & ptr_scheduler_;
		  //ar & ptr_workflow_engine_;
	  }

	  friend class boost::serialization::access;

	  virtual void print()
	  {
		  SDPA_LOG_DEBUG("The content of the JobManager is:");
		  ptr_job_man_->print();

		  SDPA_LOG_DEBUG("The content of the Scheduler is:");
		  ptr_scheduler_->print();
	  }

	  // should be overriden by the end components
	  // virtual void print_statistics (std::ostream & s) const {}

	  template <typename T>
	  T* create_workflow_engine()
	  {
	      T* pWfE = new T(this, boost::bind(&GenericDaemon::gen_id, this));
	      assert (pWfE);
              ptr_workflow_engine_ = pWfE;
              return pWfE;
	  }

	  void create_workflow_engine( IWorkflowEngine* pWfEArg ) { ptr_workflow_engine_ = pWfEArg; }
	  virtual IWorkflowEngine* workflowEngine() const { return ptr_workflow_engine_; }
	  virtual bool hasWorkflowEngine() { return ptr_workflow_engine_?true:false;}
	  virtual bool is_orchestrator() { return false; }

	  virtual const std::string url() const {return std::string();}
	  virtual const std::string masterName() const { return  std::string(); }

	  void incExtJobsCnt();
	  void decExtJobsCnt();
	  unsigned int extJobsCnt();

	  void notifyMaster(const sdpa::events::ErrorEvent::error_code_t&);
	  void notifyWorkers(const sdpa::events::ErrorEvent::error_code_t&);

  protected:

	 // observe workflow engine
	 template <typename T>
	 static void observe_submitted (const T* l, typename T::internal_id_type const & id)
	 {
             std::cerr << "activity submitted: id := " << id << std::endl;
             l->print_statistics( std::cerr );
	 }

	 template <typename T>
	 static void observe_finished (const T* l, typename T::internal_id_type const & id, std::string const &)
	 {
             std::cerr << "activity finished: id := " << id << std::endl;
             l->print_statistics( std::cerr );
	 }

	 template <typename T>
	 static void observe_failed (const T* l, typename T::internal_id_type const & id, std::string const &)
	 {
             std::cerr << "activity failed: id := " << id << std::endl;
             l->print_statistics( std::cerr );
	 }

	 template <typename T>
	 static void observe_cancelled (const T* l, typename T::internal_id_type const & id, std::string const &)
	 {
             std::cerr << "activity cancelled: id := " << id << std::endl;
             l->print_statistics( std::cerr );
	 }

	 template <typename T>
	 static void observe_executing (const T* l, typename T::internal_id_type const & id )
	 {
             std::cerr << "activity executing: id := " << id << std::endl;
             l->print_statistics( std::cerr );
	 }

	  // obsolete
	  GenericDaemon( const std::string&, seda::Stage*, seda::Stage*, IWorkflowEngine* );
	  // obsolete
	  GenericDaemon( const std::string &name, const std::string&, const std::string&, IWorkflowEngine* );

	  virtual Scheduler* create_scheduler()
	  {
	     return NULL;
	  }

	  JobManager::ptr_t ptr_job_man_;
	  Scheduler::ptr_t  ptr_scheduler_;
	  IWorkflowEngine*  ptr_workflow_engine_;

	  seda::Stage::Ptr  ptr_to_master_stage_;
	  seda::Stage::Ptr  ptr_to_slave_stage_;

	  sdpa::weak_ptr<seda::Stage> ptr_daemon_stage_;
	  //seda::Stage* ptr_daemon_stage_;
	  std::string master_;

	  sdpa::util::Config daemon_cfg_;

	  bool m_bRegistered;
	  unsigned int m_nRank;
	  sdpa::worker_id_t m_strAgentUID;

	  sdpa::util::time_type m_ullPollingInterval;
	  unsigned int m_nExternalJobs;

	  std::string m_to_master_stage_name_;
	  std::string m_to_slave_stage_name_;

	  mutex_type mtx_;
	  condition_type cond_can_stop_;
	  condition_type cond_can_start_;

	private:
	  mutable mutex_type ext_job_cnt_mtx_;

	protected:
	  bool m_bRequestsAllowed;
	  bool m_bStopped;
	  bool m_bStarted;
	  bool m_bConfigOk;
	  BackupService m_threadBkpService;
  };
}}

#endif
