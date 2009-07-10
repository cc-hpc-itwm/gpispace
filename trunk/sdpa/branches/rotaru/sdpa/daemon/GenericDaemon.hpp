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

#include <sdpa/wf/WFE_to_SDPA.hpp>
#include <sdpa/wf/SDPA_to_WFE.hpp>

namespace sdpa { namespace tests { class DaemonFSMTest_SMC; class DaemonFSMTest_BSC;}}

namespace sdpa { namespace daemon {
  class GenericDaemon : public sdpa::daemon::GenericDaemonActions,
						public sdpa::daemon::ISendEvent,
						public seda::Strategy,
						public sdpa::wf::WFE_to_SDPA {
  public:
	  GenericDaemon(const std::string &name, const std::string &outputStage);
	  virtual ~GenericDaemon();

	  virtual void perform(const seda::IEvent::Ptr &);

	  virtual void onStageStart(const std::string &stageName);
	  virtual void onStageStop(const std::string &stageName);

		//actions
	  virtual void action_configure( const sdpa::events::StartUpEvent& );
	  virtual void action_config_ok( const sdpa::events::ConfigOkEvent& );
	  virtual void action_config_nok( const sdpa::events::ConfigNokEvent& );
	  virtual void action_interrupt( const sdpa::events::InterruptEvent& );
	  virtual void action_lifesign( const sdpa::events::LifeSignEvent& );
	  virtual void action_delete_job( const sdpa::events::DeleteJobEvent& );
	  virtual void action_request_job( const sdpa::events::RequestJobEvent& );
	  virtual void action_submit_job( const sdpa::events::SubmitJobEvent& );
	  virtual void action_submit_job_ack( const sdpa::events::SubmitJobAckEvent& );
	  virtual void action_config_request( const sdpa::events::ConfigRequestEvent& );

	  virtual void sendEvent(const std::string& stageName, const sdpa::events::SDPAEvent::Ptr& e);

      // WFE_to_SDPA interface implementation
	  virtual sdpa::wf::workflow_id_t submitWorkflow(const sdpa::wf::workflow_t &workflow);
	  virtual sdpa::wf::activity_id_t submitActivity(const sdpa::wf::activity_t &activity);
	  virtual void cancelWorkflow(const sdpa::wf::workflow_id_t &workflowId); //throw (NoSuchWorkflowException);
	  virtual void cancelActivity(const sdpa::wf::activity_id_t &activityId);//throw (NoSuchActivityException) = 0;
	  virtual void workflowFinished(const sdpa::wf::workflow_id_t &workflowId);//throw (NoSuchWorkflowException) = 0;
	  virtual void workflowFailed(const sdpa::wf::workflow_id_t &workflowId); //throw (NoSuchWorkflowException) = 0;
	  virtual void workflowCanceled(const sdpa::wf::workflow_id_t &workflowId); //throw (NoSuchWorkflowException) = 0;

	  //only for testing purposes!
	  friend class sdpa::tests::DaemonFSMTest_SMC;
	  friend class sdpa::tests::DaemonFSMTest_BSC;

  protected:
	  SDPA_DECLARE_LOGGER();

	  JobManager::ptr_t 	ptr_job_man_;
	  WorkerManager::ptr_t 	ptr_worker_man_;
	  SchedulerImpl::ptr_t 	ptr_scheduler_;
	  sdpa::wf::SDPA_to_WFE* ptr_SDPA_to_WFE;

	  const std::string output_stage_;

	  void setStage(seda::Stage::Ptr stage)
	  {
		  // assert stage->strategy() == this
		  daemon_stage_ = stage;
	  }

	  seda::Stage::Ptr daemon_stage_;
  };

  /*
  class Orchestrator : public GenericDaemon {

  };
  */
}}

#endif
