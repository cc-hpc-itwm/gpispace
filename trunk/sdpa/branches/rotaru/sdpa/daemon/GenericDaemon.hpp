#ifndef SDPA_DAEMON_GENERIC_DAEMON_HPP
#define SDPA_DAEMON_GENERIC_DAEMON_HPP 1

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception_translator.hpp>

#include <seda/Strategy.hpp>

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/Scheduler.hpp>
#include <sdpa/daemon/GenericDaemonActions.hpp>

#include <sdpa/daemon/exceptions.hpp>

namespace sdpa { namespace daemon {
  class GenericDaemon : public sdpa::daemon::GenericDaemonActions, public seda::Strategy {
  public:
	  typedef std::map<sdpa::job_id_t, Job::ptr_t> job_map_t;

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

	  virtual Job::ptr_t FindJob(const sdpa::job_id_t& ) throw(JobNotFoundException) ;
	  virtual job_map_t GetJobList();

  protected:
	  // FIXME: implement as a standalone class
	  job_map_t job_map_;
	  job_map_t job_map_marked_for_del_;

	  Scheduler::ptr_t scheduler_;
  };

  /*
  class Orchestrator : public GenericDaemon {

  };
  */
}}

#endif
