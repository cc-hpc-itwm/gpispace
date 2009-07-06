#ifndef SDPA_DAEMON_GENERIC_DAEMON_HPP
#define SDPA_DAEMON_GENERIC_DAEMON_HPP 1

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception_translator.hpp>

#include <seda/Strategy.hpp>

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/Scheduler.hpp>
#include <sdpa/daemon/GenericDaemonActions.hpp>
#include <sdpa/daemon/ISendEvent.hpp>
#include <sdpa/daemon/exceptions.hpp>

namespace sdpa { namespace tests { class DaemonFSMTest_SMC; class DaemonFSMTest_BSC;}}

namespace sdpa { namespace daemon {
  class GenericDaemon : public sdpa::daemon::GenericDaemonActions,
						public sdpa::daemon::ISendEvent,
						public seda::Strategy {
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

	  virtual Job::ptr_t findJob(const sdpa::job_id_t& ) throw(JobNotFoundException) ;
	  virtual void addJob(const sdpa::job_id_t&, const Job::ptr_t& ) throw(JobNotAddedException) ;
	  virtual void deleteJob(const sdpa::job_id_t& ) throw(JobNotDeletedException) ;
	  void markJobForDeletion(const sdpa::job_id_t& job_id, const Job::ptr_t& pJob) throw(JobNotMarkedException);
	  std::vector<sdpa::job_id_t> getJobIDList();

	  Worker::ptr_t findWorker(const Worker::worker_id_t& worker_id) throw(WorkerNotFoundException);

	  virtual void sendEvent(const std::string& stageName, const sdpa::events::SDPAEvent::Ptr& e);

	  //only for testing purposes!
	  friend class sdpa::tests::DaemonFSMTest_SMC;
	  friend class sdpa::tests::DaemonFSMTest_BSC;

  protected:
	  SDPA_DECLARE_LOGGER();

    // FIXME: implement as a standalone class
    typedef std::map<sdpa::job_id_t, Job::ptr_t> job_map_t;
    typedef std::map<Worker::worker_id_t, Worker::ptr_t> worker_map_t;

    job_map_t job_map_;
    job_map_t job_map_marked_for_del_;

    worker_map_t worker_map_;
    Scheduler::ptr_t scheduler_;

    const std::string output_stage_;
  };

  /*
  class Orchestrator : public GenericDaemon {

  };
  */
}}

#endif
