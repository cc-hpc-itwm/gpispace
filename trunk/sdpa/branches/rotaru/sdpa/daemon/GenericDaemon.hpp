#ifndef SDPA_DAEMON_GENERIC_DAEMON_HPP
#define SDPA_DAEMON_GENERIC_DAEMON_HPP 1

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception_translator.hpp>

#include <seda/Stage.hpp>
#include <seda/Strategy.hpp>

#include <sdpa/daemon/SchedulerImpl.hpp>
#include <sdpa/daemon/JobManager.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/GenericDaemonActions.hpp>
#include <sdpa/daemon/ISendEvent.hpp>

#include <sdpa/wf/Gwes2Sdpa.hpp>
#include <sdpa/wf/Sdpa2Gwes.hpp>

#include <sdpa/events/SubmitJobEvent.hpp>
//#include <gwes/Sdpa2Gwes.h>

namespace sdpa { namespace tests { class DaemonFSMTest_SMC; class DaemonFSMTest_BSC;}}

namespace sdpa { namespace daemon {
  class GenericDaemon : public sdpa::daemon::GenericDaemonActions,
						public sdpa::daemon::ISendEvent,
						public seda::Strategy,
						public sdpa::wf::Gwes2Sdpa {
  public:
	  typedef sdpa::shared_ptr<GenericDaemon> ptr_t;
	  virtual ~GenericDaemon();

	  // API
	  static ptr_t create( const std::string &name_prefix, const std::string &outputStage, sdpa::wf::Sdpa2Gwes* pArgSdpa2Gwes = NULL);
	  static void start(GenericDaemon::ptr_t daemon );

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
	  virtual void sendEvent(const std::string& stageName, const sdpa::events::SDPAEvent::Ptr& e);

      // Gwes2Sdpa interface implementation
	  virtual sdpa::wf::workflow_id_t submitWorkflow(const sdpa::wf::workflow_t &workflow);
	  virtual sdpa::wf::activity_id_t submitActivity(const sdpa::wf::activity_t &activity);
	  virtual void cancelWorkflow(const sdpa::wf::workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException);
	  virtual void cancelActivity(const sdpa::wf::activity_id_t &activityId) throw (sdpa::daemon::NoSuchActivityException);
	  virtual void workflowFinished(const sdpa::wf::workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException);
	  virtual void workflowFailed(const sdpa::wf::workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException);
	  virtual void workflowCanceled(const sdpa::wf::workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException);

	  Worker::ptr_t findWorker(const Worker::worker_id_t& worker_id) throw(WorkerNotFoundException);
	  void addWorker(const  Worker::ptr_t );

	  std::string master()const { return master_;}

	  //only for testing purposes!
	  friend class sdpa::tests::DaemonFSMTest_SMC;
	  friend class sdpa::tests::DaemonFSMTest_BSC;

	  virtual const std::string output_stage() const { return output_stage_ ; }

  protected:
	  SDPA_DECLARE_LOGGER();

	  GenericDaemon(const std::string &name, const std::string &outputStage, sdpa::wf::Sdpa2Gwes*  pSdpa2Gwes);

	  JobManager::ptr_t ptr_job_man_;
	  Scheduler::ptr_t 	ptr_scheduler_;
	  sdpa::wf::Sdpa2Gwes*  ptr_Sdpa2Gwes_;

	  void setStage(seda::Stage* stage)
	  {
		  // assert stage->strategy() == this
		  daemon_stage_ = stage;
	  }

	  const std::string output_stage_;
	  seda::Stage* daemon_stage_;
	  std::string master_;
  };

  /*
  class Orchestrator : public GenericDaemon {

  };
  */
}}

#endif
