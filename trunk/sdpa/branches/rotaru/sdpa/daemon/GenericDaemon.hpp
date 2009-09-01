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
	  static ptr_t create( const std::string &name_prefix, const std::string &outputStage );

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

      // Gwes2Sdpa interface implementation
	  virtual sdpa::wf::workflow_id_t submitWorkflow(const sdpa::wf::workflow_t &workflow);
	  virtual sdpa::wf::activity_id_t submitActivity(const sdpa::wf::activity_t &activity);
	  virtual void cancelWorkflow(const sdpa::wf::workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException);
	  virtual void cancelActivity(const sdpa::wf::activity_id_t &activityId) throw (sdpa::daemon::NoSuchActivityException);
	  virtual void workflowFinished(const sdpa::wf::workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException);
	  virtual void workflowFailed(const sdpa::wf::workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException);
	  virtual void workflowCanceled(const sdpa::wf::workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException);

	  //only for testing purposes!
	  friend class sdpa::tests::DaemonFSMTest_SMC;
	  friend class sdpa::tests::DaemonFSMTest_BSC;

  protected:
	  SDPA_DECLARE_LOGGER();

	  GenericDaemon(const std::string &name, const std::string &outputStage);

	  JobManager::ptr_t 	ptr_job_man_;
	  WorkerManager::ptr_t 	ptr_worker_man_;
	  SchedulerImpl::ptr_t 	ptr_scheduler_;
	  sdpa::wf::Sdpa2Gwes*  ptr_Sdpa2Gwes;

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
