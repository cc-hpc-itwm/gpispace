/*
 * =====================================================================================
 *
 *       Filename:  GenericDaemon.hpp
 *
 *    Description:  Generic daemon header file
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
#ifndef SDPA_DAEMON_GENERIC_DAEMON_HPP
#define SDPA_DAEMON_GENERIC_DAEMON_HPP 1

#include <seda/Strategy.hpp>

#include <sdpa/sdpa-config.hpp>

#include <sdpa/util/Config.hpp>
#include <sdpa/daemon/SchedulerImpl.hpp>
#include <sdpa/daemon/JobManager.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/GenericDaemonActions.hpp>

#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/WorkerRegistrationAckEvent.hpp>
#include <sdpa/events/ConfigReplyEvent.hpp>

#include <sdpa/types.hpp>

#ifdef USE_BOOST_SC
#  include <boost/statechart/state_machine.hpp>
#  include <boost/statechart/simple_state.hpp>
#  include <boost/statechart/custom_reaction.hpp>
#  include <boost/statechart/transition.hpp>
#  include <boost/statechart/exception_translator.hpp>
#endif

namespace sdpa { namespace tests { class DaemonFSMTest_SMC; class DaemonFSMTest_BSC;}}

namespace sdpa { namespace daemon {
  class GenericDaemon : public sdpa::daemon::GenericDaemonActions,
						public sdpa::daemon::IComm,
						public seda::Strategy,
						public gwes::Gwes2Sdpa {
  public:
	  typedef sdpa::shared_ptr<GenericDaemon> ptr_t;
	  virtual ~GenericDaemon();

	  // API
	  static void create_daemon_stage(const GenericDaemon::ptr_t& ptr_daemon );
	  static void start(const GenericDaemon::ptr_t& daemon );
	  static void start(const GenericDaemon::ptr_t& ptr_daemon, sdpa::util::Config::ptr_t ptrCfg);
	  void stop();

	  virtual void perform(const seda::IEvent::Ptr&);
	  virtual void handleDaemonEvent(const seda::IEvent::Ptr& pEvent);
	  virtual void handleJobEvent(const seda::IEvent::Ptr& pEvent);

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

	  // management event handlers
	  virtual void handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent*);
	  virtual void handleConfigReplyEvent(const sdpa::events::ConfigReplyEvent*);

	  // job event handlers
	  virtual void handleSubmitJobAckEvent(const sdpa::events::SubmitJobAckEvent* pEvent);
	  virtual void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent* );
	  virtual void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
	  virtual void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );
	  virtual void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent* );
	  virtual void handleJobFailedAckEvent(const sdpa::events::JobFailedAckEvent* );
	  virtual void handleQueryJobStatusEvent(const sdpa::events::QueryJobStatusEvent* );
	  virtual void handleCancelJobEvent(const sdpa::events::CancelJobEvent*);
	  virtual void handleRetrieveResultsEvent(const sdpa::events::RetrieveJobResultsEvent* ptr );

	  virtual void sendEvent(const sdpa::events::SDPAEvent::Ptr& e);
	  virtual void sendEvent(seda::Stage* ptrOutStage, const sdpa::events::SDPAEvent::Ptr& e);

      // Gwes2Sdpa interface implementation
	  //virtual workflow_id_t submitWorkflow(const workflow_t &workflow);
	  virtual gwes::activity_id_t submitActivity(gwes::activity_t &activity);

	  //virtual void cancelWorkflow(const workflow_id_t &workflowId) throw (NoSuchWorkflowException);
	  virtual void cancelActivity(const gwes::activity_id_t &activityId) throw (gwes::Gwes2Sdpa::NoSuchActivity);

	  virtual void workflowFinished(const gwes::workflow_id_t &workflowId, const gwdl::workflow_result_t&) throw (gwes::Gwes2Sdpa::NoSuchWorkflow);
	  virtual void workflowFailed(const gwes::workflow_id_t &workflowId, const gwdl::workflow_result_t&) throw (gwes::Gwes2Sdpa::NoSuchWorkflow);
	  virtual void workflowCanceled(const gwes::workflow_id_t &workflowId, const gwdl::workflow_result_t&) throw (gwes::Gwes2Sdpa::NoSuchWorkflow);

	  virtual void jobFinished(std::string workerName, const job_id_t &);
	  virtual void jobFailed(std::string workerName, const job_id_t &);
	  virtual void jobCancelled(std::string workerName, const job_id_t &);

	  //virtual void configure_network();
	  virtual void configure_network( std::string daemonUrl, std::string masterName = "", std::string masterUrl = "" );
	  virtual void shutdown_network();

	  Worker::ptr_t findWorker(const Worker::worker_id_t& worker_id) throw(WorkerNotFoundException);
	  void addWorker(const  Worker::ptr_t );

	  std::string master()const { return master_;}
	  void setMaster( std::string masterName ){ master_=masterName;}

	  const std::string& name() const { return Strategy::name(); }

	  JobManager::ptr_t jobManager() const { return ptr_job_man_; }

	  virtual seda::Stage* daemon_stage() { return daemon_stage_; }
	  virtual seda::Stage* to_master_stage() const { return ptr_to_master_stage_ ; }
	  virtual seda::Stage* to_slave_stage() const { return ptr_to_slave_stage_ ; }

	  virtual void set_to_slave_stage(seda::Stage* argStage) { ptr_to_slave_stage_= argStage; }

	  virtual sdpa::Sdpa2Gwes* gwes() const { return ptr_Sdpa2Gwes_; }
	  virtual bool is_registered() const { return m_bRegistered;}

	  sdpa::util::Config* cfg() const { return ptr_daemon_cfg_.get();}

  protected:
	  SDPA_DECLARE_LOGGER();

	  GenericDaemon(const std::string&, seda::Stage*, seda::Stage*, sdpa::Sdpa2Gwes*);
	  GenericDaemon(const std::string &name, const std::string&, const std::string&,sdpa::Sdpa2Gwes*);

	  GenericDaemon( const std::string &name, sdpa::Sdpa2Gwes*  pArgSdpa2Gwes);

	  JobManager::ptr_t ptr_job_man_;
	  Scheduler::ptr_t 	ptr_scheduler_;
	  sdpa::Sdpa2Gwes*  ptr_Sdpa2Gwes_;

	  void setStage(seda::Stage* stage)
	  {
		  // assert stage->strategy() == this
		  daemon_stage_ = stage;
	  }

	  seda::Stage* ptr_to_master_stage_;
	  seda::Stage* ptr_to_slave_stage_;

	  seda::Stage* daemon_stage_;
	  std::string master_;

	  sdpa::util::Config::ptr_t ptr_daemon_cfg_;
	  bool m_bRegistered;
  };
}}

#endif
