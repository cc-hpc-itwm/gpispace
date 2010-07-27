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
#include <seda/comm/delivery_service.hpp>
#include <seda/comm/ServiceThread.hpp>

#include <sdpa/sdpa-config.hpp>

#include <sdpa/util/Config.hpp>
#include <sdpa/daemon/SchedulerImpl.hpp>
#include <sdpa/daemon/JobManager.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/GenericDaemonActions.hpp>

#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/WorkerRegistrationAckEvent.hpp>
#include <sdpa/events/ConfigReplyEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

#include <sdpa/types.hpp>

#ifdef USE_BOOST_SC
#  include <boost/statechart/state_machine.hpp>
#  include <boost/statechart/simple_state.hpp>
#  include <boost/statechart/custom_reaction.hpp>
#  include <boost/statechart/transition.hpp>
#  include <boost/statechart/exception_translator.hpp>
#endif

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
						public IDaemon,
						boost::noncopyable
{
  public:
	  typedef boost::recursive_mutex mutex_type;
	  typedef boost::unique_lock<mutex_type> lock_type;

	  typedef sdpa::shared_ptr<GenericDaemon> ptr_t;
	  virtual ~GenericDaemon();

	  // API
	  static void create_daemon_stage(const GenericDaemon::ptr_t& ptr_daemon );
	  static void start(const GenericDaemon::ptr_t& daemon );
	  static void start(const GenericDaemon::ptr_t& ptr_daemon, sdpa::util::Config::ptr_t ptrCfg);
	  void stop();

	  virtual void perform(const seda::IEvent::Ptr&);

	  virtual void onStageStart(const std::string &stageName);
	  virtual void onStageStop(const std::string &stageName);

	  // daemon actions
	  virtual void action_configure( const sdpa::events::StartUpEvent& );
	  virtual void action_config_ok( const sdpa::events::ConfigOkEvent& );
	  virtual void action_config_nok( const sdpa::events::ConfigNokEvent& );
	  virtual void action_interrupt( const sdpa::events::InterruptEvent& );
	  virtual void action_lifesign( const sdpa::events::LifeSignEvent& );
	  virtual void action_delete_job( const sdpa::events::DeleteJobEvent& );
	  virtual void action_request_job( const sdpa::events::RequestJobEvent& );
	  virtual void action_submit_job( const sdpa::events::SubmitJobEvent& );
	  virtual void action_config_request( const sdpa::events::ConfigRequestEvent& );
	  virtual void action_register_worker(const sdpa::events::WorkerRegistrationEvent& );
	  virtual void action_error_event(const sdpa::events::ErrorEvent& );

	  virtual void handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent*);
	  virtual void handleConfigReplyEvent(const sdpa::events::ConfigReplyEvent*);

	  // job event handlers
	  virtual void handleSubmitJobAckEvent(const sdpa::events::SubmitJobAckEvent* pEvent);
	  virtual void handleCancelJobEvent(const sdpa::events::CancelJobEvent*);
	  virtual void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent* );
	  virtual void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
	  virtual void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );
	  virtual void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent* );
	  virtual void handleJobFailedAckEvent(const sdpa::events::JobFailedAckEvent* );
	  virtual void handleQueryJobStatusEvent(const sdpa::events::QueryJobStatusEvent* );
	  virtual void handleRetrieveJobResultsEvent(const sdpa::events::RetrieveJobResultsEvent* ptr );

	  virtual void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& e);
	  virtual void sendEventToMaster(const sdpa::events::SDPAEvent::Ptr& e, std::size_t retries = 0, unsigned long timeout = 1); // 0 retries, 1 second timeout
	  virtual void sendEventToSlave(const sdpa::events::SDPAEvent::Ptr& e, std::size_t retries = 0, unsigned long timeout = 1); // 0 retries, 1 second timeout
	  virtual bool acknowledge(const sdpa::events::SDPAEvent::message_id_type &mid);

      // WE interface
	  virtual void submit(const id_type & id, const encoded_type & );
	  virtual bool cancel(const id_type & id, const reason_type & reason);
	  virtual bool finished(const id_type & id, const result_type & result);
	  virtual bool failed(const id_type & id, const result_type & result);
	  virtual bool cancelled(const id_type & id);

	  virtual void submitWorkflow(const id_type& id, const encoded_type& );
	  virtual void cancelWorkflow(const id_type& workflowId, const std::string& reason);
	  virtual void workerJobFailed(const job_id_t&, const std::string& result /*or reason*/) ;
	  virtual void workerJobFinished(const job_id_t & id, const result_type& result );
	  virtual void workerJobCancelled(const job_id_t& id);
	  virtual void activityCancelled(const id_type& id, const std::string& data);

	  //virtual void configure_network();
	  virtual void configure_network( std::string daemonUrl, std::string masterName = "", std::string masterUrl = "" );
	  virtual void shutdown_network();

	  virtual const Worker::worker_id_t& findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException);

	  Worker::ptr_t findWorker(const Worker::worker_id_t& worker_id) throw(WorkerNotFoundException);
	  virtual void addWorker( const Worker::worker_id_t& workerId, unsigned int rank ) throw (WorkerAlreadyExistException);

	  std::string master()const { return master_;}
	  void setMaster( std::string masterName ){ master_=masterName;}

	  const std::string& name() const { return Strategy::name(); }

	  JobManager::ptr_t jobManager() const { return ptr_job_man_; }

	  Job::ptr_t& findJob(const sdpa::job_id_t& job_id ) throw(JobNotFoundException);

	  virtual seda::Stage* daemon_stage() { return daemon_stage_; }
	  virtual seda::Stage* to_master_stage() const { return ptr_to_master_stage_ ; }
	  virtual seda::Stage* to_slave_stage() const { return ptr_to_slave_stage_ ; }

	  virtual void set_to_slave_stage(seda::Stage* argStage) { ptr_to_slave_stage_= argStage; }

	  virtual IWorkflowEngine* workflowEngine() const { return ptr_workflow_engine_; }
	  virtual bool is_registered() const { return m_bRegistered;}

	  sdpa::util::Config* cfg() const { return ptr_daemon_cfg_.get();}
	  const unsigned int& rank() const { return m_nRank; }
	  unsigned int& rank() { return m_nRank; }

	  //boost::bind(&sdpa_daemon::gen_id, this))
	  std::string gen_id() { JobId jobId; return jobId.str(); }

	  void jobFailed(const job_id_t&, const std::string& reason);
	  const we::preference_t& getJobPreferences(const sdpa::job_id_t& jobId) const throw (NoJobPreferences);

	  virtual bool requestsAllowed();

	  template <class Archive>
	  void serialize(Archive& ar, const unsigned int)
	  {
		  ar & ptr_job_man_;
		  ar & ptr_scheduler_;
		  //ar & ptr_workflow_engine_;
	  }

	  friend class boost::serialization::access;
	  friend class sdpa::tests::WorkerSerializationTest;

	  virtual void print() {
	  		  ptr_job_man_->print();
	  		  ptr_scheduler_->print();
	  }


	  // should be overriden by the end components
	  //virtual void print_statistics (std::ostream & s) const {}

	  template <typename T>
	  T* create_workflow_engine()
	  {
		  T* pWfE = new T(this, boost::bind(&GenericDaemon::gen_id, this));
//		  pWfE->sig_submitted.connect( &GenericDaemon::observe_submitted<T> );
//		  pWfE->sig_finished.connect(  &GenericDaemon::observe_finished<T> );
//		  pWfE->sig_failed.connect(    &GenericDaemon::observe_failed<T> );
//		  pWfE->sig_cancelled.connect( &GenericDaemon::observe_cancelled<T> );
//		  pWfE->sig_executing.connect( &GenericDaemon::observe_executing<T> );

		  return pWfE;
	  }

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

	  SDPA_DECLARE_LOGGER();

	  GenericDaemon( const std::string&, seda::Stage*, seda::Stage*, IWorkflowEngine* );
	  GenericDaemon( const std::string &name, const std::string&, const std::string&, IWorkflowEngine* );
	  GenericDaemon( const std::string name = sdpa::daemon::ORCHESTRATOR, IWorkflowEngine* pArgSdpa2Gwes = NULL );

	  virtual Scheduler* create_scheduler()
	  {
		  return new SchedulerImpl(this);
	  }

	  JobManager::ptr_t ptr_job_man_;
	  Scheduler::ptr_t 	ptr_scheduler_;
	  IWorkflowEngine*  ptr_workflow_engine_;

	  void setStage(seda::Stage* stage)
	  {
            assert (stage);
		  daemon_stage_ = stage;
	  }

	  seda::Stage* ptr_to_master_stage_;
	  seda::Stage* ptr_to_slave_stage_;

	  seda::Stage* daemon_stage_;
	  std::string master_;

	  sdpa::util::Config::ptr_t ptr_daemon_cfg_;
	  bool m_bRegistered;
	  unsigned int m_nRank;
	  unsigned int m_nExternalJobs;

	private:
	  typedef seda::comm::delivery_service<sdpa::events::SDPAEvent::Ptr, sdpa::events::SDPAEvent::message_id_type, seda::Stage> sdpa_msg_delivery_service;
	  seda::comm::ServiceThread service_thread_;
	  sdpa_msg_delivery_service delivery_service_;

	  void messageDeliveryFailed(sdpa::events::SDPAEvent::Ptr);

	  mutable mutex_type mtx_;
  };
}}

#endif
